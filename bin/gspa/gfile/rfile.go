package gfile

import (
	"gspa"
	"gspa/gcs"
	"io"
	"os"
	"sync"

	"github.com/gammazero/deque"
)

const (
	//request ids
	Download         gspa.ReqId = 0x7F70
	StartDownloading gspa.ReqId = 0x7F71
	Downloading      gspa.ReqId = 0x7F72
	Upload           gspa.ReqId = 0x7F73
	Uploading        gspa.ReqId = 0x7F74
	UploadCompleted  gspa.ReqId = 0x7F75
	UploadBackup     gspa.ReqId = 0x7F76

	//file open flags
	FILE_OPEN_TRUNCACTED  uint32 = 1
	FILE_OPEN_APPENDED    uint32 = 2
	FILE_OPEN_SHARE_READ  uint32 = 4
	FILE_OPEN_SHARE_WRITE uint32 = 8

	//error code
	CANNOT_OPEN_LOCAL_FILE_FOR_WRITING int32 = -1
	CANNOT_OPEN_LOCAL_FILE_FOR_READING int32 = -2
	FILE_BAD_OPERATION                 int32 = -3
	FILE_DOWNLOADING_INTERRUPTED       int32 = -4

	//others
	STREAM_CHUNK_SIZE  uint32 = 10240
	MAX_FILES_STREAMED uint32 = 32
	INVALID_FILE_SIZE  uint64 = 0xffffffffffffffff
)

type CStreamingFile struct {
	ah               *gcs.CAsyncHandler
	cs               sync.Mutex
	m_vContext       deque.Deque[*cContext]
	m_maxDownloading uint32
}

type DDownload func(sender *CStreamingFile, err *gspa.ErrInfo)
type DTransferring func(sender *CStreamingFile, transferred uint64)

type cContext struct {
	gspa.ErrInfo
	Download     DDownload
	Transferring DTransferring
	Discarded    gcs.DDiscarded
	Se           gcs.DServerException
	Uploading    bool
	FileSize     uint64
	Flags        uint32
	LocalFile    string
	FilePath     string
	QueueOk      bool
	Sent         bool
	InitSize     int64
	File         *os.File
	Promise      chan interface{}
}

func (c *cContext) GetFilePos() int64 {
	if c.File == nil {
		panic("Unexpected operation")
	}
	pos, _ := c.File.Seek(0, 1)
	return pos
}

func (c *cContext) IsOpen() bool {
	return (c.File != nil)
}

func (c *cContext) HasError() bool {
	return (c.ErrCode != 0 || len(c.ErrMsg) > 0)
}

func (c *cContext) CloseFile() {
	if c.File != nil {
		if !c.Uploading && c.HasError() {
			if c.InitSize == -1 {
				c.File.Close()
				os.Remove(c.LocalFile)
			} else {
				c.File.Sync()
				c.File.Truncate(c.InitSize)
				c.File.Close()
			}
		} else {
			c.File.Close()
		}
		c.File = nil
	}
}

func (c *cContext) OpenLocalRead() {
	if c.File != nil {
		panic("Bad operation!")
	}
	fm := (os.ModeExclusive | os.ModePerm)
	if (c.Flags & FILE_OPEN_SHARE_READ) == FILE_OPEN_SHARE_READ {
		fm = os.ModePerm
	}
	var err error
	c.File, err = os.OpenFile(c.LocalFile, os.O_RDONLY, fm)
	if err != nil {
		c.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING
		c.ErrMsg = err.Error()
		c.File = nil
		return
	} else if c.File == nil {
		c.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING
		c.ErrMsg = "Cannot open local file for reading"
		return
	}
	fi, _ := c.File.Stat()
	c.FileSize = uint64(fi.Size())
}

func (c *cContext) OpenLocalWrite() {
	if c.File != nil {
		panic("Bad operation!")
	}
	fm := (os.ModeExclusive | os.ModePerm)
	if (c.Flags & FILE_OPEN_SHARE_WRITE) == FILE_OPEN_SHARE_WRITE {
		fm = os.ModePerm
	}
	var err error
	existing := false
	mode := (os.O_WRONLY | os.O_CREATE | os.O_EXCL)
	c.File, err = os.OpenFile(c.LocalFile, mode, fm)
	if err != nil || c.File == nil {
		existing = true
		mode = (os.O_WRONLY | os.O_CREATE)
		if (c.Flags & FILE_OPEN_TRUNCACTED) == FILE_OPEN_TRUNCACTED {
			mode |= os.O_TRUNC
		} else if (c.Flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED {
			mode |= os.O_APPEND
		}
		c.File, err = os.OpenFile(c.LocalFile, mode, fm)
		if err != nil {
			c.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING
			c.ErrMsg = err.Error()
			c.File = nil
			return
		} else if c.File == nil {
			c.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING
			c.ErrMsg = "Cannot open local file for writing"
			return
		}
	}
	if existing {
		pos, _ := c.File.Seek(0, io.SeekCurrent)
		c.InitSize = pos
	}
}

func (sf *CStreamingFile) requestProcessed(reqId gspa.ReqId, buffer *gspa.CUQueue) {
	switch reqId {
	case Download:
		res := buffer.LoadInt()
		errMsg := buffer.LoadString()
		var dl DDownload
		var cxt *cContext
		sf.cs.Lock()
		if sf.m_vContext.Len() > 0 {
			cxt = sf.m_vContext.Front()
			dl = cxt.Download
			cxt.ErrCode = res
			cxt.ErrMsg = *errMsg
		}
		sf.cs.Unlock()
		if dl != nil {
			dl(sf, &cxt.ErrInfo)
		}
		if cxt != nil {
			sf.cs.Lock()
			cxt.CloseFile()
			sf.m_vContext.PopFront()
			sf.cs.Unlock()
		}
		sf.postProcessing(0, 0)
	case StartDownloading:
		fileSize := buffer.LoadULong()
		localFile := buffer.LoadString()
		remoteFile := buffer.LoadString()
		flags := buffer.LoadUInt()
		initSize := buffer.LoadLong()
		sf.cs.Lock()
		if sf.m_vContext.Len() == 0 {
			c := cContext{Uploading: false, FileSize: INVALID_FILE_SIZE, Flags: flags, InitSize: -1}
			c.LocalFile = *localFile
			c.FilePath = *remoteFile
			c.OpenLocalWrite()
			c.InitSize = initSize
			sf.m_vContext.PushBack(&c)
		}
		sf.cs.Unlock()
		cxt := sf.m_vContext.Front()
		cxt.FileSize = fileSize
		if cxt.InitSize < 0 {
			cxt.InitSize = 0
		}
		if cxt.GetFilePos() > initSize {
			cxt.File.Truncate(initSize)
		}
	case Downloading:
		var progress DTransferring
		var downloaded uint64
		var cxt *cContext
		sf.cs.Lock()
		if sf.m_vContext.Len() > 0 {
			cxt = sf.m_vContext.Front()
			progress = cxt.Transferring
			bytes := buffer.PopBytes()
			cxt.File.Write(bytes)
			downloaded = uint64(cxt.GetFilePos())
		}
		sf.cs.Unlock()
		if progress != nil {
			progress(sf, downloaded)
		}
		buffer.SetSize(0)
	case UploadBackup:
		break
	case Upload:
		var cxt *cContext
		res := buffer.LoadInt()
		errMsg := *buffer.LoadString()
		if res != 0 || len(errMsg) > 0 {
			sf.cs.Lock()
			if sf.m_vContext.Len() > 0 {
				cxt = sf.m_vContext.Front()
				cxt.InitSize = buffer.LoadLong()
				cxt.ErrMsg = errMsg
				cxt.ErrCode = res
			}
			sf.cs.Unlock()
		} else {
			sf.cs.Lock()
			defer sf.cs.Unlock()
			if sf.m_vContext.Len() > 0 {
				cxt = sf.m_vContext.Front()
				cxt.InitSize = buffer.LoadLong()
				cxt.QueueOk = sf.ah.GetClientQueue().StartJob()
				queue_enabled := sf.ah.GetClientQueue().IsAvailable()
				sb := gspa.MakeBuffer()
				if queue_enabled {
					sb.Save(cxt.FilePath, cxt.Flags, cxt.FileSize, cxt.InitSize)
					sf.ah.SendRequest(UploadBackup, sb, nil, cxt.Discarded, nil)
					sb.SetSize(0)
				}
				sb.Realloc(STREAM_CHUNK_SIZE)
				bytes := sb.GetInternalBuffer()
				n, err := cxt.File.Read(bytes)
				for err == nil && n > 0 {
					sb.SetSize(uint32(n))
					if !sf.ah.SendRequest(Uploading, sb, nil, cxt.Discarded, nil) {
						cxt.ErrCode = sf.ah.GetErrorCode()
						cxt.ErrMsg = sf.ah.GetErrorMsg()
						break
					}
					sb.SetSize(0)
					if sf.ah.GetBytesInSendingBuffer() > 40*STREAM_CHUNK_SIZE {
						break
					}
					n, err = cxt.File.Read(bytes)
				}
				if !cxt.HasError() && uint32(n) < STREAM_CHUNK_SIZE {
					cxt.Sent = true
					if !sf.ah.SendRequest(UploadCompleted, nil, nil, cxt.Discarded, nil) {
						cxt.ErrCode = sf.ah.GetErrorCode()
						cxt.ErrMsg = sf.ah.GetErrorMsg()
					}
					if cxt.QueueOk {
						sf.ah.GetClientQueue().EndJob()
					}
				}
			}
		}
		if cxt != nil && cxt.HasError() {
			cxt.CloseFile()
			if cxt.Download != nil {
				cxt.Download(sf, &cxt.ErrInfo)
			}
			sf.cs.Lock()
			sf.m_vContext.PopFront()
			sf.cs.Unlock()
			if cxt.QueueOk {
				sf.ah.GetClientQueue().AbortJob()
			}
			sf.postProcessing(0, 0)
		}
		break
	case Uploading:
		var res int32
		var errMsg string
		var ctx *cContext
		var progress DTransferring
		uploaded := buffer.LoadLong()
		if buffer.GetSize() >= 8 {
			res = buffer.LoadInt()
			errMsg = *buffer.LoadString()
		}
		sf.cs.Lock()
		if sf.m_vContext.Len() > 0 {
			context := sf.m_vContext.Front()
			progress = context.Transferring
			if uploaded < 0 || res != 0 || len(errMsg) > 0 {
				context.ErrCode = res
				context.ErrMsg = errMsg
				ctx = context
			} else if !context.Sent {
				sb := gspa.MakeBuffer()
				sb.Realloc(STREAM_CHUNK_SIZE)
				bytes := sb.GetInternalBuffer()
				n, err := context.File.Read(bytes)
				if err != nil {
					context.ErrCode = CANNOT_OPEN_LOCAL_FILE_FOR_READING
					context.ErrMsg = err.Error()
					ctx = context
				} else if n > 0 {
					sb.SetSize(uint32(n))
					if !sf.ah.SendRequest(Uploading, sb, nil, context.Discarded, nil) {
						context.ErrCode = sf.ah.GetErrorCode()
						context.ErrMsg = sf.ah.GetErrorMsg()
						ctx = context
					}
				}
				if err == nil && uint32(n) < STREAM_CHUNK_SIZE {
					context.Sent = true
					if !sf.ah.SendRequest(UploadCompleted, sb, nil, context.Discarded, nil) {
						context.ErrCode = sf.ah.GetErrorCode()
						context.ErrMsg = sf.ah.GetErrorMsg()
						ctx = context
					}
				}
			}
		}
		sf.cs.Unlock()
		if ctx != nil && ctx.HasError() {
			ctx.CloseFile()
			if ctx.Download != nil {
				ctx.Download(sf, &ctx.ErrInfo)
			}
			sf.cs.Lock()
			sf.m_vContext.PopFront()
			sf.cs.Unlock()
			sf.postProcessing(0, 0)
		} else if progress != nil {
			progress(sf, uint64(uploaded))
		}
	case UploadCompleted:
		var upl DDownload
		var cxt *cContext
		sf.cs.Lock()
		if sf.m_vContext.Len() > 0 {
			cxt = sf.m_vContext.Front()
			if cxt.IsOpen() {
				upl = cxt.Download
			} else {
				cxt.Sent = false
				cxt.QueueOk = false
			}
		}
		sf.cs.Unlock()
		if upl != nil {
			upl(sf, &gspa.ErrInfo{})
		}
		if cxt != nil && cxt.IsOpen() {
			sf.cs.Lock()
			cxt.CloseFile()
			sf.m_vContext.PopFront()
			sf.cs.Unlock()
		}
		sf.postProcessing(0, 0)
	}
}

func (sf *CStreamingFile) mergeTo(to *gcs.CAsyncHandler) {
	fTo := FindFile(to)
	fTo.cs.Lock()
	defer fTo.cs.Unlock()
	count := fTo.m_vContext.Len()
	pos := count
	for n := 0; n < count; n++ {
		ctx := fTo.m_vContext.At(n)
		if !ctx.IsOpen() && !ctx.HasError() {
			pos = n
			break
		}
	}

	sf.cs.Lock()
	for sf.m_vContext.Len() > 0 {
		ctx := sf.m_vContext.PopBack()
		fTo.m_vContext.Insert(pos, ctx)
	}
	sf.cs.Unlock()

	if count == 0 && fTo.m_vContext.Len() > 0 {
		to.PostProcessing(0, 0)
		to.DoEcho()
	}
}

func (sf *CStreamingFile) safeClean() {
	sf.cs.Lock()
	vContext := sf.m_vContext
	sf.m_vContext = deque.Deque[*cContext]{}
	sf.cs.Unlock()
	for vContext.Len() > 0 {
		cx := vContext.PopFront()
		if cx.IsOpen() {
			cx.File.Close()
			cx.File = nil
		}
		if cx.Discarded != nil {
			cx.Discarded(sf.ah, sf.ah.GetCurrentRequestID() == gspa.Cancel)
		}
	}
}

func (sf *CStreamingFile) postProcessing(hint uint32, data uint64) {
	d := uint32(0)
	sf.cs.Lock()
	defer sf.cs.Unlock()
	count := sf.m_vContext.Len()
	for n := 0; n < count; n++ {
		if d >= sf.m_maxDownloading {
			break
		}
		it := sf.m_vContext.At(n)
		if it.IsOpen() {
			if it.Uploading {
				break
			} else {
				d++
				continue
			}
		}
		if it.HasError() {
			continue
		}
		if it.Uploading {
			it.OpenLocalRead()
			if !it.HasError() {
				buffer := gspa.MakeBuffer().Save(it.FilePath, it.Flags, it.FileSize)
				defer buffer.Recycle()
				if !sf.ah.SendRequest(Upload, buffer, nil, it.Discarded, it.Se) {
					it.ErrCode = sf.ah.GetErrorCode()
					if it.ErrCode == 0 {
						it.ErrCode = gcs.SESSION_CLOSED_BEFORE
						it.ErrMsg = gcs.SESSION_CLOSED_BEFORE_ERR_MSG
					} else {
						it.ErrMsg = sf.ah.GetErrorMsg()
					}
					continue
				}
				break
			}
		} else {
			it.OpenLocalWrite()
			if !it.HasError() {
				buffer := gspa.MakeBuffer().Save(it.LocalFile, it.FilePath, it.Flags, it.InitSize)
				defer buffer.Recycle()
				if !sf.ah.SendRequest(Download, buffer, nil, it.Discarded, it.Se) {
					it.ErrCode = sf.ah.GetErrorCode()
					if it.ErrCode == 0 {
						it.ErrCode = gcs.SESSION_CLOSED_BEFORE
						it.ErrMsg = gcs.SESSION_CLOSED_BEFORE_ERR_MSG
					} else {
						it.ErrMsg = sf.ah.GetErrorMsg()
					}
					continue
				}
				d++
			}
		}
	}
	for sf.m_vContext.Len() > 0 {
		it := sf.m_vContext.Front()
		if it.HasError() {
			it.CloseFile()
			cb := it.Download
			if cb != nil {
				sf.cs.Unlock()
				defer sf.cs.Lock()
				cb(sf, &it.ErrInfo)
			}
			sf.m_vContext.PopFront()
		} else {
			break
		}
	}
}

func (sf *CStreamingFile) getFilesOpened() uint32 {
	opened := uint32(0)
	size := sf.m_vContext.Len()
	for n := 0; n < size; n++ {
		it := sf.m_vContext.At(n)
		if it.IsOpen() {
			opened++
		} else if it.HasError() {
			break
		}
	}
	return opened
}

func (sf *CStreamingFile) GetFilesStreamed() uint32 {
	sf.cs.Lock()
	fs := sf.m_maxDownloading
	sf.cs.Unlock()
	return fs
}

func (sf *CStreamingFile) SetFilesStreamed(max uint32) *CStreamingFile {
	if max == 0 {
		max = 1
	} else if max > MAX_FILES_STREAMED {
		max = MAX_FILES_STREAMED
	}
	sf.cs.Lock()
	sf.m_maxDownloading = max
	sf.cs.Unlock()
	return sf
}

func (sf *CStreamingFile) GetFilesQueued() uint32 {
	sf.cs.Lock()
	fq := uint32(sf.m_vContext.Len())
	sf.cs.Unlock()
	return fq
}

func (sf *CStreamingFile) GetFileSize() uint64 {
	file_size := INVALID_FILE_SIZE
	sf.cs.Lock()
	if sf.m_vContext.Len() > 0 {
		file_size = sf.m_vContext.Front().FileSize
	}
	sf.cs.Unlock()
	return file_size
}

func (sf *CStreamingFile) Cancel() uint32 {
	canceled := uint32(0)
	sf.cs.Lock()
	defer sf.cs.Unlock()
	for sf.m_vContext.Len() > 0 {
		back := sf.m_vContext.Back()
		if back.IsOpen() {
			sf.ah.Interrupt(gcs.DEFAULT_INTERRUPT_OPTION)
			break
		}
		if back.Discarded != nil {
			go func() {
				back.Discarded(sf.ah, true)
			}()
		}
		sf.m_vContext.PopBack()
		canceled++
	}
	return canceled
}

func (sf *CStreamingFile) setCancelError(ch chan interface{}, reqId gspa.ReqId, canceled bool) {
	var err gcs.SocketError
	err.ReqId = reqId
	err.Before = false
	if canceled {
		err.ErrCode = gcs.REQUEST_CANCELED
		err.ErrMsg = gcs.REQUEST_CANCELED_ERR_MSG
	} else {
		err.ErrCode = sf.ah.GetErrorCode()
		if err.ErrCode != 0 {
			err.ErrMsg = sf.ah.GetErrorMsg()
		} else {
			err.ErrCode = gcs.SESSION_CLOSED_AFTER
			err.ErrMsg = gcs.SESSION_CLOSED_AFTER_ERR_MSG
		}
	}
	ch <- err
}

func (sf *CStreamingFile) pp(context *cContext, localFile string, remoteFile string, progress DTransferring) {
	context.LocalFile = localFile
	context.FilePath = remoteFile
	context.Transferring = progress
	context.Promise = make(chan interface{})
	context.Download = func(sender *CStreamingFile, err *gspa.ErrInfo) {
		context.Promise <- err
	}
	context.Discarded = func(sender *gcs.CAsyncHandler, canceled bool) {
		var reqId gspa.ReqId
		if context.Uploading {
			reqId = Upload
		} else {
			reqId = Download
		}
		sf.setCancelError(context.Promise, reqId, canceled)
	}
	context.Se = func(sender *gcs.CAsyncHandler, reqId gspa.ReqId, ec int32, em string, where string) {
		var err gcs.ServerError
		err.ReqId = reqId
		err.ErrCode = ec
		err.ErrMsg = em
		err.Location = where
		context.Promise <- err
	}
	sf.cs.Lock()
	defer sf.cs.Unlock()
	sf.m_vContext.PushBack(context)
	filesOpened := sf.getFilesOpened()
	if sf.m_maxDownloading > filesOpened {
		sf.ah.PostProcessing(0, 0)
		if filesOpened == 0 {
			sf.ah.DoEcho() //make sure WaitAll works correctly
		}
	}
}

func (sf *CStreamingFile) Transfer(upload bool, localFile string, remoteFile string, progress DTransferring, flags ...uint32) <-chan interface{} {
	if len(localFile) == 0 || len(remoteFile) == 0 {
		panic("File paths cannot be empty")
	}
	mode := FILE_OPEN_TRUNCACTED
	if len(flags) > 0 {
		mode = flags[0]
	}
	c := cContext{Uploading: upload, FileSize: INVALID_FILE_SIZE, Flags: mode, InitSize: -1}
	sf.pp(&c, localFile, remoteFile, progress)
	return c.Promise
}

//find an instance of CStreamingFile from a tied/given base handler ah
func FindFile(ah *gcs.CAsyncHandler) *CStreamingFile {
	csFile.Lock()
	defer csFile.Unlock()
	if val, ok := gMapFile[ah]; ok {
		return val
	}
	return nil
}

//implementation of interface gcs.ITie::GetHandler()
func (sf *CStreamingFile) GetHandler() *gcs.CAsyncHandler {
	return sf.ah
}

var (
	csFile   sync.Mutex
	gMapFile = make(map[*gcs.CAsyncHandler]*CStreamingFile)
)

//implementation of interface gcs.ITie::Tie
//tie an instnace of CStreamingFile sf with a given instence of base handler gcs.CAsyncHandler ah
func (sf *CStreamingFile) Tie(ah *gcs.CAsyncHandler) bool {
	if ah != nil && sf != nil {
		ah.SetTie(sf) //tie streaming file with ah
		//register four events, RequestProcessed, MergeTo, Clean and PostProcessing
		ah.SetOnRequestProcessed(sf.requestProcessed).SetOnMergeTo(sf.mergeTo).SetOnClean(sf.safeClean).SetOnPostProcessing(sf.postProcessing)
		sf.cs.Lock()
		sf.m_maxDownloading = 1
		sf.ah = ah //for gcs.ITie::GetHandler()
		sf.cs.Unlock()
		csFile.Lock()
		//register sf so that we can use FindFile to find an instance of CStreamingFile later
		gMapFile[ah] = sf
		csFile.Unlock()
		return true
	}
	return false
}

//implementation of interface gcs.ITie::Untie
//this required method will be automatically called by a socket pool when the pool is about to be shutdown
func (sf *CStreamingFile) Untie() {
	if sf != nil {
		ah := sf.ah
		if ah != nil {
			csFile.Lock()
			//unregister fi
			delete(gMapFile, ah)
			csFile.Unlock()
			ah.SetTie(nil)
		}
	}
}
