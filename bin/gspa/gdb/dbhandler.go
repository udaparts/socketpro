package gdb

import (
	"gspa"
	"gspa/gcs"
	"sync"
)

type SQLExeInfo struct {
	gspa.ErrInfo
	Affected int64
	Oks      uint32
	Fails    uint32
	LastId   interface{}
}

type CDBHandler struct {
	ah            *gcs.CAsyncHandler
	csSending     sync.Mutex
	cs            sync.Mutex
	m_blob        *gspa.CUQueue
	m_indexRowset uint64
	m_mapRowset   map[uint64]*cRowsetHandler
	m_queueOk     bool
	m_bCallReturn bool
	m_lastReqId   gspa.ReqId
	ms            ManagementSystem
	m_parameters  uint32
	m_outputs     uint32
	m_flags       uint32
	m_strConn     string
	m_vData       []interface{}
	m_vColInfo    *CDBColumnInfoArray
	m_mapOdbcInfo map[uint16]interface{}
}

type DResult func(sender *CDBHandler, err *gspa.ErrInfo)
type DExecuteResult func(sender *CDBHandler, err *SQLExeInfo)
type DRowsetHeader func(sender *CDBHandler)
type DRows func(sender *CDBHandler, proc bool, vData []interface{})

type cRowsetHandler struct {
	rh  DRowsetHeader
	row DRows
	bh  DRowsetHeader
}

func (db *CDBHandler) Open(conn string, flags uint32, handler ...DResult) <-chan interface{} {
	sb := gspa.MakeBuffer()
	defer sb.Recycle()
	db.cs.Lock()
	db.m_flags = flags
	db.m_strConn = conn
	db.cs.Unlock()
	sb.Save(conn, flags)
	res := make(chan interface{})
	res = db.ah.Send(Open, sb, func(ar *gcs.CAsyncResult) {
		db = FindDB(ar.Sender)
		ei := gspa.ErrInfo{ErrCode: ar.Buffer.LoadInt(), ErrMsg: *ar.Buffer.LoadString()}
		db.cs.Lock()
		db.m_lastReqId = Open
		if ei.ErrCode == 0 {
			db.m_strConn = ei.ErrMsg
			ei.ErrMsg = ""
		} else {
			db.m_strConn = ""
		}
		db.ms = ManagementSystem(ar.Buffer.LoadInt())
		db.m_parameters = 0
		db.m_outputs = 0
		db.cs.Unlock()
		var cb DResult
		if len(handler) > 0 {
			cb = handler[0]
		}
		if cb != nil {
			cb(db, &ei)
		}
		res <- &ei
		close(res)
	})
	return res
}

func (db *CDBHandler) Close(handler ...DResult) <-chan interface{} {
	res := make(chan interface{})
	res = db.ah.Send(Close, nil, func(ar *gcs.CAsyncResult) {
		db = FindDB(ar.Sender)
		ei := gspa.ErrInfo{ErrCode: ar.Buffer.LoadInt(), ErrMsg: *ar.Buffer.LoadString()}
		db.cs.Lock()
		db.m_strConn = ""
		db.m_lastReqId = Close
		db.cs.Unlock()
		var cb DResult
		if len(handler) > 0 {
			cb = handler[0]
		}
		if cb != nil {
			cb(db, &ei)
		}
		res <- &ei
		close(res)
	})
	return res
}

func (db *CDBHandler) Prepare(sql string, handler DResult, vPInfo ...[]CParameterInfo) <-chan interface{} {
	sb := gspa.MakeBuffer()
	defer sb.Recycle()
	sb.Save(sql)
	var vP []CParameterInfo
	if len(vPInfo) > 0 {
		vP = vPInfo[0]
	}
	sb.Save(uint32(len(vP)))
	for _, p := range vP {
		p.SaveTo(sb)
	}
	res := make(chan interface{})
	res = db.ah.Send(Prepare, sb, func(ar *gcs.CAsyncResult) {
		db = FindDB(ar.Sender)
		ei := gspa.ErrInfo{ErrCode: ar.Buffer.LoadInt(), ErrMsg: *ar.Buffer.LoadString()}
		parameters := ar.Buffer.LoadUInt()
		db.cs.Lock()
		db.m_bCallReturn = false
		db.m_lastReqId = Prepare
		db.m_parameters = (parameters & 0xffff)
		db.m_outputs = (parameters >> 16)
		db.cs.Unlock()
		if handler != nil {
			handler(db, &ei)
		}
		res <- &ei
		close(res)
	})
	return res
}

func (db *CDBHandler) BeginTrans(handler DResult, isolation ...TransactionIsolation) <-chan interface{} {
	sb := gspa.MakeBuffer()
	defer sb.Recycle()
	iso := ReadCommited
	if len(isolation) > 0 {
		iso = isolation[0]
	}
	res := make(chan interface{})
	cq := db.ah.GetClientQueue()
	sb.Save(int32(iso))
	db.csSending.Lock()
	defer db.csSending.Unlock()
	db.cs.Lock()
	sb.Save(db.m_strConn, db.m_flags)
	db.cs.Unlock()
	db.m_queueOk = (cq.IsAvailable() && cq.StartJob())
	res = db.ah.Send(BeginTrans, sb, func(ar *gcs.CAsyncResult) {
		db = FindDB(ar.Sender)
		ei := gspa.ErrInfo{ErrCode: ar.Buffer.LoadInt(), ErrMsg: *ar.Buffer.LoadString()}
		db.cs.Lock()
		db.m_lastReqId = BeginTrans
		if ei.ErrCode == 0 {
			db.m_strConn = ei.ErrMsg
			ei.ErrMsg = ""
		}
		db.ms = ManagementSystem(ar.Buffer.LoadInt())
		db.cs.Unlock()
		if handler != nil {
			handler(db, &ei)
		}
		res <- &ei
		close(res)
	})
	return res
}

func (db *CDBHandler) EndTrans(handler DResult, plan ...RollbackPlan) <-chan interface{} {
	sb := gspa.MakeBuffer()
	defer sb.Recycle()
	rb := Default
	if len(plan) > 0 {
		rb = plan[0]
	}
	sb.Save(int32(rb))
	res := make(chan interface{})
	cq := db.ah.GetClientQueue()
	db.csSending.Lock()
	defer db.csSending.Unlock()
	res = db.ah.Send(EndTrans, sb, func(ar *gcs.CAsyncResult) {
		db = FindDB(ar.Sender)
		ei := gspa.ErrInfo{ErrCode: ar.Buffer.LoadInt(), ErrMsg: *ar.Buffer.LoadString()}
		db.cs.Lock()
		db.m_lastReqId = EndTrans
		db.cs.Unlock()
		if handler != nil {
			handler(db, &ei)
		}
		res <- &ei
		close(res)
	})
	if db.m_queueOk {
		cq.EndJob()
		db.m_queueOk = false
	}
	return res
}

func (db *CDBHandler) process(ch chan interface{}, er DExecuteResult, ar *gcs.CAsyncResult, reqId gspa.ReqId, index uint64) {
	err := new(SQLExeInfo)
	q := ar.Buffer
	err.Affected = q.LoadLong()
	err.ErrCode = q.LoadInt()
	err.ErrMsg = *q.LoadString()
	err.LastId = q.LoadObject()
	fail_ok := q.LoadULong()
	err.Oks = uint32(fail_ok)
	err.Fails = uint32(fail_ok >> 32)
	db = FindDB(ar.Sender)
	db.cs.Lock()
	db.m_lastReqId = reqId
	_, f := db.m_mapRowset[index]
	if f {
		delete(db.m_mapRowset, index)
	}
	db.cs.Unlock()
	if er != nil {
		er(db, err)
	}
	ch <- err
}

func (db *CDBHandler) Execute(sql string, row DRows, rh DRowsetHeader, handler ...DExecuteResult) <-chan interface{} {
	rowset := (row != nil)
	meta := (rh != nil)
	lastId := true
	sb := gspa.MakeBuffer()
	defer sb.Recycle()
	sb.Save(sql, rowset, meta, lastId)
	res := make(chan interface{})
	db.csSending.Lock()
	defer db.csSending.Unlock()
	index := gcs.GetCallIndex()
	if rowset || meta {
		db.cs.Lock()
		db.m_mapRowset[index] = &cRowsetHandler{rh: rh, row: row}
		db.cs.Unlock()
	}
	sb.Save(index)
	res = db.ah.Send(Execute, sb, func(ar *gcs.CAsyncResult) {
		var er DExecuteResult
		if len(handler) > 0 {
			er = handler[0]
		}
		db.process(res, er, ar, Execute, index)
	})
	return res
}

func (db *CDBHandler) ExecuteEx(sql string, params *[]interface{}, row DRows, rh DRowsetHeader, handler DExecuteResult, delimiter ...string) <-chan interface{} {
	rowset := (row != nil)
	meta := (rh != nil)
	lastId := true
	sb := gspa.MakeBuffer()
	defer sb.Recycle()
	sb.SaveString(&sql)
	if params == nil {
		sb.SaveUInt(0)
	} else {
		sb.SaveUInt(uint32(len(*params)))
		for _, p := range *params {
			sb.SaveObject(p)
		}
	}
	sb.Save(rowset)
	sep := ";"
	if len(delimiter) > 0 && len(delimiter[0]) > 0 {
		sep = delimiter[0]
	}
	sb.SaveString(&sep)
	sb.Save(meta, lastId)
	res := make(chan interface{})
	db.csSending.Lock()
	defer db.csSending.Unlock()
	index := gcs.GetCallIndex()
	if rowset || meta {
		db.cs.Lock()
		db.m_mapRowset[index] = &cRowsetHandler{rh: rh, row: row}
		db.cs.Unlock()
	}
	sb.Save(index)
	res = db.ah.Send(ExecuteEx, sb, func(ar *gcs.CAsyncResult) {
		db.process(res, handler, ar, ExecuteEx, index)
	})
	return res
}

func (db *CDBHandler) setSocketErrorBefore(ch chan interface{}, reqId gspa.ReqId) {
	var err gcs.SocketError
	ah := db.ah
	err.ReqId = reqId
	err.Before = true
	err.ErrCode = ah.GetErrorCode()
	if err.ErrCode != 0 {
		err.ErrMsg = ah.GetErrorMsg()
	} else {
		err.ErrCode = gcs.SESSION_CLOSED_BEFORE
		err.ErrMsg = gcs.SESSION_CLOSED_BEFORE_ERR_MSG
	}
	go func() {
		ch <- err
	}()
}

func (db *CDBHandler) ExecuteBatch(isolation TransactionIsolation, sql string, params *[]interface{}, row DRows, rh DRowsetHeader,
	batchHedaer DRowsetHeader, delimiter string, plan RollbackPlan, er DExecuteResult, vPInfo ...CParameterInfo) <-chan interface{} {
	rowset := (row != nil)
	meta := (rh != nil)
	lastId := true
	sb := gspa.MakeBuffer()
	defer sb.Recycle()
	queueOk := false
	cq := db.ah.GetClientQueue()
	res := make(chan interface{})
	db.csSending.Lock()
	defer db.csSending.Unlock()
	if params != nil && len(*params) > 0 {
		queueOk = (cq.IsAvailable() && cq.StartJob())
		if !db.sendParametersData(sb, params) {
			db.clean()
			db.setSocketErrorBefore(res, ExecuteParameters)
			return res
		}
		sb.SetSize(0)
	}
	if len(delimiter) == 0 {
		delimiter = ";"
	}
	sb.Save(sql, delimiter, int32(isolation), int32(plan), rowset, meta, lastId)
	index := gcs.GetCallIndex()
	db.cs.Lock()
	if rowset || meta || batchHedaer != nil {
		db.m_mapRowset[index] = &cRowsetHandler{rh: rh, row: row, bh: batchHedaer}
	}
	sb.Save(db.m_strConn, db.m_flags)
	db.cs.Unlock()
	sb.Save(index)
	sb.Save(uint32(len(vPInfo)))
	for _, p := range vPInfo {
		p.SaveTo(sb)
	}
	res = db.ah.Send(ExecuteBatch, sb, func(ar *gcs.CAsyncResult) {
		db.process(res, er, ar, ExecuteBatch, index)
	})
	if queueOk {
		cq.EndJob()
	}
	return res
}

func (db *CDBHandler) ExecuteParameter(params *[]interface{}, row DRows, rh DRowsetHeader, handler ...DExecuteResult) <-chan interface{} {
	rowset := (row != nil)
	meta := (rh != nil)
	lastId := true
	sb := gspa.MakeBuffer()
	defer sb.Recycle()
	queueOk := false
	cq := db.ah.GetClientQueue()
	res := make(chan interface{})
	db.csSending.Lock()
	defer db.csSending.Unlock()
	if params != nil && len(*params) > 0 {
		queueOk = (cq.IsAvailable() && cq.StartJob())
		if !db.sendParametersData(sb, params) {
			db.clean()
			db.setSocketErrorBefore(res, ExecuteParameters)
			return res
		}
		sb.SetSize(0)
	}
	sb.Save(rowset, meta, lastId)
	index := gcs.GetCallIndex()
	if rowset || meta {
		db.cs.Lock()
		db.m_mapRowset[index] = &cRowsetHandler{rh: rh, row: row}
		db.cs.Unlock()
	}
	sb.Save(index)
	res = db.ah.Send(ExecuteParameters, sb, func(ar *gcs.CAsyncResult) {
		var er DExecuteResult
		if len(handler) > 0 {
			er = handler[0]
		}
		db.process(res, er, ar, ExecuteParameters, index)
	})
	if queueOk {
		cq.EndJob()
	}
	return res
}

func (db *CDBHandler) send(sb *gspa.CUQueue, firstRow *bool) bool {
	if sb.GetSize() > 0 {
		if *firstRow {
			*firstRow = false
			if !db.ah.SendRequest(BeginRows, sb, nil, nil, nil) {
				return false
			}
		} else {
			if !db.ah.SendRequest(Transferring, sb, nil, nil, nil) {
				return false
			}
		}
		sb.SetSize(0)
	} else if *firstRow {
		*firstRow = false
		if !db.ah.SendRequest(BeginRows, nil, nil, nil, nil) {
			return false
		}
	}
	return true
}

func (db *CDBHandler) sendBlob(sb *gspa.CUQueue) bool {
	if sb.GetSize() == 0 {
		return true
	}
	var offset uint32
	var q gspa.CUQueue
	start := true
	count := sb.GetSize()
	for offset < count {
		bytes := sb.GetBytes(offset, DEFAULT_BIG_FIELD_CHUNK_SIZE)
		bs := uint32(len(bytes))
		offset += bs
		q.SetBuffer(bytes, bs)
		if start {
			if !db.ah.SendRequest(StartBLOB, &q, nil, nil, nil) {
				sb.Discard(offset)
				return false
			}
			start = false
		} else {
			if !db.ah.SendRequest(Chunk, &q, nil, nil, nil) {
				sb.Discard(offset)
				return false
			}
		}
		if count-offset <= DEFAULT_BIG_FIELD_CHUNK_SIZE {
			sb.Discard(offset)
			break
		}
	}
	if !db.ah.SendRequest(EndBLOB, sb, nil, nil, nil) {
		return false
	}
	sb.SetSize(0)
	return true
}
func (db *CDBHandler) sendArray(sb *gspa.CUQueue, size uint32, vt interface{}, firstRow *bool) bool {
	if size > DEFAULT_BIG_FIELD_CHUNK_SIZE {
		if !db.send(sb, firstRow) {
			return false
		}
		len := sb.Save(uint32(0)).SaveObject(vt).GetSize() - 4
		*(*uint32)(sb.GetBuffer()) = len
		if !db.sendBlob(sb) {
			return false
		}
	} else {
		sb.SaveObject(vt)
	}
	return true
}

func (db *CDBHandler) sendParametersData(sb *gspa.CUQueue, params *[]interface{}) bool {
	if params != nil {
		size := len(*params)
		if size > 0 {
			firstRow := true
			vData := *params
			for n := 0; n < size; n++ {
				vt := vData[n]
				if vt == nil {
					sb.SaveObject(vt)
				} else {
					switch vt.(type) {
					case string:
						s := vt.(string)
						if !db.sendArray(sb, uint32(len(s)), vt, &firstRow) {
							return false
						}
					case *string:
						s := vt.(*string)
						if !db.sendArray(sb, uint32(len(*s)), vt, &firstRow) {
							return false
						}
					case gspa.AStr:
						s := vt.(gspa.AStr)
						if !db.sendArray(sb, uint32(len(s)), vt, &firstRow) {
							return false
						}
					case *gspa.AStr:
						s := vt.(*gspa.AStr)
						if !db.sendArray(sb, uint32(len(*s)), vt, &firstRow) {
							return false
						}
					case []byte:
						s := vt.([]byte)
						if !db.sendArray(sb, uint32(len(s)), vt, &firstRow) {
							return false
						}
					default:
						sb.SaveObject(vt)
					}
				}
				if sb.GetSize() >= DEFAULT_RECORD_BATCH_SIZE {
					if !db.send(sb, &firstRow) {
						return false
					}
				}
			}
			if !db.send(sb, &firstRow) {
				return false
			}
		}
	}
	return true
}

func (db *CDBHandler) GetDBManagementSystem() ManagementSystem {
	db.cs.Lock()
	ms := db.ms
	db.cs.Unlock()
	return ms
}

func (db *CDBHandler) GetParameters() uint32 {
	db.cs.Lock()
	parameters := db.m_parameters
	db.cs.Unlock()
	return parameters
}

func (db *CDBHandler) GetOutputs() uint32 {
	db.cs.Lock()
	outputs := db.m_outputs
	db.cs.Unlock()
	return outputs
}

func (db *CDBHandler) GetCallReturn() bool {
	db.cs.Lock()
	b := db.m_bCallReturn
	db.cs.Unlock()
	return b
}

func (db *CDBHandler) IsOpened() bool {
	db.cs.Lock()
	b := (len(db.m_strConn) > 0)
	db.cs.Unlock()
	return b
}

func (db *CDBHandler) GetConnection() string {
	db.cs.Lock()
	s := db.m_strConn
	db.cs.Unlock()
	return s
}

func (db *CDBHandler) GetColumnInfo() *CDBColumnInfoArray {
	db.cs.Lock()
	a := db.m_vColInfo
	db.cs.Unlock()
	return a
}

func (db *CDBHandler) GetODBCInfo(infoType uint16) interface{} {
	db.cs.Lock()
	v, found := db.m_mapOdbcInfo[infoType]
	db.cs.Unlock()
	if found {
		return v
	}
	return nil
}

func (db *CDBHandler) clean() {
	db.m_lastReqId = 0
	if db.m_vColInfo == nil || len(*db.m_vColInfo) > 0 {
		db.m_vColInfo = new(CDBColumnInfoArray)
	}
	if len(db.m_vData) > 0 {
		db.m_vData = make([]interface{}, 0)
	}
	if len(db.m_mapRowset) > 0 {
		db.m_mapRowset = make(map[uint64]*cRowsetHandler)
	}
	db.m_blob.SetSize(0)
	if db.m_blob.GetMaxSize() > DEFAULT_BIG_FIELD_CHUNK_SIZE {
		db.m_blob.Realloc(DEFAULT_BIG_FIELD_CHUNK_SIZE)
	}
}

func (db *CDBHandler) safeClean() {
	db.cs.Lock()
	defer db.cs.Unlock()
	db.clean()
}

func (db *CDBHandler) requestProcessed(reqId gspa.ReqId, buffer *gspa.CUQueue) {
	switch reqId {
	case ParameterPosition:
		db.cs.Lock()
		db.m_bCallReturn = false
		db.cs.Unlock()
	case SqlBatchHeader:
		var header DRowsetHeader
		res := buffer.LoadInt()
		em := buffer.LoadString()
		ms := buffer.LoadInt()
		params := buffer.LoadUInt()
		index := buffer.LoadULong()
		db.cs.Lock()
		if db.m_vColInfo == nil || len(*db.m_vColInfo) > 0 {
			db.m_vColInfo = new(CDBColumnInfoArray)
		}
		db.m_lastReqId = SqlBatchHeader
		db.m_parameters = (params & 0xffff)
		db.m_outputs = 0
		if res == 0 {
			db.m_strConn = *em
		}
		db.ms = ManagementSystem(ms)
		cbs, f := db.m_mapRowset[index]
		if f {
			header = cbs.bh
		}
		db.cs.Unlock()
		if header != nil {
			header(db)
		}
	case RowsetHeader:
		var header DRowsetHeader
		if len(db.m_vData) > 0 {
			db.m_vData = make([]interface{}, 0)
		}
		outputs := uint32(0)
		db.cs.Lock()
		db.m_vColInfo = new(CDBColumnInfoArray)
		db.m_vColInfo.LoadFrom(buffer)
		db.m_indexRowset = buffer.LoadULong()
		if buffer.GetSize() > 0 {
			outputs = buffer.LoadUInt()
		}
		if outputs == 0 && len(*db.m_vColInfo) > 0 {
			cbs, f := db.m_mapRowset[db.m_indexRowset]
			if f {
				header = cbs.rh
			}
		}
		db.cs.Unlock()
		if header != nil {
			header(db)
		}
	case BeginRows:
		db.m_blob.SetSize(0)
		if len(db.m_vData) > 0 {
			db.m_vData = make([]interface{}, 0)
		}
		if buffer.GetSize() > 0 {
			db.m_indexRowset = buffer.LoadULong()
		}
	case Transferring:
		for buffer.GetSize() > 0 {
			db.m_vData = append(db.m_vData, buffer.LoadObject())
		}
	case CallReturn:
		db.m_vData = make([]interface{}, 1)
		db.m_vData[0] = buffer.LoadObject()
		db.cs.Lock()
		db.m_bCallReturn = true
		db.cs.Unlock()
	case EndRows, OutputParameter:
		if buffer.GetSize() > 0 || len(db.m_vData) > 0 {
			for buffer.GetSize() > 0 {
				db.m_vData = append(db.m_vData, buffer.LoadObject())
			}
			var row DRows
			db.cs.Lock()
			if reqId == OutputParameter {
				if db.m_outputs == 0 {
					db.m_outputs += uint32(len(db.m_vData))
				}
				db.m_vColInfo = new(CDBColumnInfoArray)
			}
			cbs, f := db.m_mapRowset[db.m_indexRowset]
			if f {
				row = cbs.row
			}
			db.cs.Unlock()
			if row != nil {
				row(db, reqId == OutputParameter, db.m_vData)
			}
		}
		if len(db.m_vData) > 0 {
			db.m_vData = make([]interface{}, 0)
		}
	case StartBLOB:
		if buffer.GetSize() > 0 {
			db.m_blob.SetSize(0)
			len := buffer.LoadUInt()
			if len != gspa.STRING_NULL_END && len > db.m_blob.GetMaxSize() {
				db.m_blob.Realloc(len)
			}
			db.m_blob.PushBytes(buffer.PopBytes())
			buffer.SetSize(0)
		}
	case Chunk:
		if buffer.GetSize() > 0 {
			db.m_blob.PushBytes(buffer.PopBytes())
			buffer.SetSize(0)
		}
	case EndBLOB:
		if buffer.GetSize() > 0 || db.m_blob.GetSize() > 0 {
			db.m_blob.PushBytes(buffer.PopBytes())
			buffer.SetSize(0)
			len := *(*uint32)(db.m_blob.GetBuffer(2))
			if len >= BLOB_LENGTH_NOT_AVAILABLE {
				//length should be reset if BLOB length not available from server side at beginning
				len = db.m_blob.GetSize() - 6
				*(*uint32)(db.m_blob.GetBuffer(2)) = len
			}
			db.m_vData = append(db.m_vData, db.m_blob.LoadObject())
			db.m_blob.SetSize(0)
		}
	case ODBC_SQLGetInfo:
		db.cs.Lock()
		defer db.cs.Unlock()
		if len(db.m_mapOdbcInfo) > 0 {
			db.m_mapOdbcInfo = make(map[uint16]interface{})
		}
		for buffer.GetSize() > 0 {
			db.m_mapOdbcInfo[buffer.LoadUShort()] = buffer.LoadObject()
		}
	}
}

func (db *CDBHandler) allProcessed(reqId gspa.ReqId) {
	db.cs.Lock()
	if len(db.m_vData) > 0 {
		db.m_vData = make([]interface{}, 0)
	}
	if len(db.m_mapRowset) > 0 {
		db.m_mapRowset = make(map[uint64]*cRowsetHandler)
	}
	db.cs.Unlock()
}

func (db *CDBHandler) mergeTo(to *gcs.CAsyncHandler) {
	dbTo := FindDB(to)
	dbTo.cs.Lock()
	defer dbTo.cs.Unlock()
	db.cs.Lock()
	if len(db.m_mapRowset) > 0 {
		for k, v := range db.m_mapRowset {
			dbTo.m_mapRowset[k] = v
		}
		db.m_mapRowset = make(map[uint64]*cRowsetHandler)
	}
	db.cs.Unlock()
}

//implementation of interface gcs.ITie::GetHandler()
func (db *CDBHandler) GetHandler() *gcs.CAsyncHandler {
	return db.ah
}

var (
	csDB   sync.Mutex
	gMapDB = make(map[*gcs.CAsyncHandler]*CDBHandler)
)

//implementation of interface gcs.ITie::Tie
//tie an instnace of CDBHandler db with a given instence of base handler gcs.CAsyncHandler ah
func (db *CDBHandler) Tie(ah *gcs.CAsyncHandler) bool {
	if ah != nil && db != nil {
		db.m_blob = gspa.MakeBuffer()
		db.m_mapRowset = make(map[uint64]*cRowsetHandler)
		db.m_mapOdbcInfo = make(map[uint16]interface{})
		db.m_vColInfo = new(CDBColumnInfoArray)
		ah.SetTie(db) //tie db with ah
		//register four events, RequestProcessed, AllRequestsProcessed, MergeTo and Clean
		ah.SetOnRequestProcessed(db.requestProcessed).SetOnAllRequestsProcessed(db.allProcessed).SetOnMergeTo(db.mergeTo).SetOnClean(db.safeClean)
		db.cs.Lock()
		db.ah = ah //for gcs.ITie::GetHandler()
		db.cs.Unlock()
		csDB.Lock()
		//register db so that we can use FindDB to find an instance of CDBHandler later
		gMapDB[ah] = db
		csDB.Unlock()
		return true
	}
	return false
}

//implementation of interface gcs.ITie::Untie
//this required method will be automatically called by a socket pool when the pool is about to be shutdown
func (db *CDBHandler) Untie() {
	if db != nil {
		ah := db.ah
		if ah != nil {
			db.cs.Lock()
			//put db.m_blob into memory pool for reuse
			db.m_blob.Recycle()
			db.cs.Unlock()
			csDB.Lock()
			//unregister db
			delete(gMapDB, ah)
			csDB.Unlock()
			ah.SetTie(nil)
		}
	}
}

//find an instance of CDBHandler from a tied/given base handler ah
func FindDB(ah *gcs.CAsyncHandler) *CDBHandler {
	csDB.Lock()
	defer csDB.Unlock()
	if val, ok := gMapDB[ah]; ok {
		return val
	}
	return nil
}
