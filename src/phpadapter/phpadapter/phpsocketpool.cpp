#include "stdafx.h"
#include "phpsocketpool.h"
#include "phpsocket.h"

namespace PA {

	const char* CPhpSocketPool::NOT_INITIALIZED = "Socket pool object not initialized";

	CPhpSocketPool::CPhpSocketPool() : m_nSvsId(0), Handler(nullptr), m_errCode(0), m_pt(NotMS) {
	}

	CPhpSocketPool::~CPhpSocketPool() {
		if (Handler) {
			switch (m_nSvsId) {
			case SPA::Sqlite::sidSqlite:
			case SPA::Mysql::sidMysql:
			case SPA::Odbc::sidOdbc:
				delete Db;
				break;
			case SPA::Queue::sidQueue:
				delete Queue;
				break;
			case SPA::SFile::sidFile:
				delete File;
				break;
			default:
				delete Handler;
				break;
			}
		}
	}

	Php::Value CPhpSocketPool::NewSlave(Php::Parameters &params) {
		if (!Handler) {
			throw Php::Exception(NOT_INITIALIZED);
		}
		if (m_pt != Master) {
			throw Php::Exception("Cannot create a slave pool from a non-master pool");
		}
		std::string defaultDb(m_defaultDb);
		bool autoConn = Handler->GetAutoConn();
		unsigned int recvTimeout = Handler->GetRecvTimeout();
		unsigned int connTimeout = Handler->GetConnTimeout();
		size_t args = params.size();
		if (args > 3) {
			connTimeout = (unsigned int)params[3].numericValue();
		}
		if (args > 2) {
			recvTimeout = (unsigned int)params[2].numericValue();
		}
		if (args > 1) {
			autoConn = params[1].boolValue();
		}
		if (args > 0) {
			defaultDb = params[0].stringValue();
		}
		Trim(defaultDb);
		return Php::Object((SPA_CS_NS + PHP_SOCKET_POOL).c_str(), (int64_t)m_nSvsId, defaultDb, autoConn, (int64_t)recvTimeout, (int64_t)connTimeout, true);
	}

	void CPhpSocketPool::ShutdownPool(Php::Parameters &params) {
		if (Handler) {
			Handler->ShutdownPool();
		}
		else {
			throw Php::Exception(NOT_INITIALIZED);
		}
	}

	Php::Value CPhpSocketPool::DisconnectAll(Php::Parameters &params) {
		bool ok = true;
		if (Handler) {
			ok = Handler->DisconnectAll();
		}
		else {
			throw Php::Exception(NOT_INITIALIZED);
		}
		return ok;
	}

	void CPhpSocketPool::__construct(Php::Parameters &params) {
		bool slave = false;
		bool autoConn = true;
		unsigned int recvTimeout = SPA::ClientSide::DEFAULT_RECV_TIMEOUT;
		unsigned int connTimeout = SPA::ClientSide::DEFAULT_CONN_TIMEOUT;
		unsigned int id = (unsigned int)params[0].numericValue();
		if (id < SPA::sidChat || (id > SPA::sidODBC && id <= SPA::sidReserved)) {
			throw Php::Exception("A valid unsigned int required for service id");
		}
		if (id == SPA::sidHTTP) {
			throw Php::Exception("No support to HTTP/websocket at client side");
		}
		size_t args = params.size();
		if (args > 5) {
			slave = (unsigned int)params[5].boolValue();
		}
		if (args > 4) {
			connTimeout = (unsigned int)params[4].numericValue();
		}
		if (args > 3) {
			recvTimeout = (unsigned int)params[3].numericValue();
		}
		if (args > 2) {
			autoConn = params[2].boolValue();
		}
		if (args > 1) {
			m_defaultDb = params[1].stringValue();
		}
		Trim(m_defaultDb);
		switch (id) {
		case SPA::Mysql::sidMysql:
		case SPA::Odbc::sidOdbc:
		case SPA::Sqlite::sidSqlite:
			if (m_defaultDb.size()) {
				std::wstring dfltDb = SPA::Utilities::ToWide(m_defaultDb.c_str(), m_defaultDb.size());
				if (slave) {
					Db = new CSQLMaster::CSlavePool(dfltDb.c_str(), recvTimeout, id);
					m_pt = Slave;
				}
				else {
					Db = new CSQLMaster(dfltDb.c_str(), recvTimeout, id);
					m_pt = Master;
				}
				Db->SetConnTimeout(connTimeout);
				Db->SetAutoConn(autoConn);
			}
			else {
				Db = new CPhpDbPool(autoConn, recvTimeout, connTimeout, id);
				m_pt = NotMS;
			}
			Db->DoSslServerAuthentication = [this](CPhpDbPool *pool, CClientSocket * cs)->bool {
				return this->DoSSLAuth(cs);
			};
			Db->SocketPoolEvent = [this](CPhpDbPool *pool, tagSocketPoolEvent spe, CDBHandler *handler) {
				SPA::CAutoLock al(this->m_cs);
				if (this->m_pe.isCallable()) {
					if (handler) {
						CPhpDb *h = new CPhpDb(pool, handler, false);
						this->m_pe((int)spe, Php::Object((SPA_CS_NS + PHP_ASYNC_HANDLER).c_str(), h), this);
					}
					else {
						this->m_pe((int)spe, nullptr, this);
					}
				}
			};

			break;
		case SPA::Queue::sidQueue:
			if (slave) {
				throw Php::Exception("CAsyncQueue handler doesn't support slave pool");
			}
			else if (m_defaultDb.size()) {
				throw Php::Exception("CAsyncQueue handler doesn't support master pool");
			}
			Queue = new CPhpQueuePool(autoConn, recvTimeout, connTimeout, id);
			Queue->DoSslServerAuthentication = [this](CPhpQueuePool *pool, CClientSocket * cs)->bool {
				return this->DoSSLAuth(cs);
			};
			Queue->SocketPoolEvent = [this](CPhpQueuePool *pool, tagSocketPoolEvent spe, CAsyncQueue *handler) {
				SPA::CAutoLock al(this->m_cs);
				if (this->m_pe.isCallable()) {
					if (handler) {
						CPhpQueue *h = new CPhpQueue(pool, handler, false);
						this->m_pe((int)spe, Php::Object((SPA_CS_NS + PHP_QUEUE_HANDLER).c_str(), h), this);
					}
					else {
						this->m_pe((int)spe, nullptr, this);
					}
				}
			};
			m_pt = NotMS;
			break;
		case SPA::SFile::sidFile:
			if (slave) {
				throw Php::Exception("CStreamingFile handler doesn't support slave pool");
			}
			else if (m_defaultDb.size()) {
				throw Php::Exception("CStreamingFile handler doesn't support master pool");
			}
			File = new CPhpFilePool(autoConn, recvTimeout, connTimeout, id);
			File->DoSslServerAuthentication = [this](CPhpFilePool *pool, CClientSocket * cs)->bool {
				return this->DoSSLAuth(cs);
			};
			File->SocketPoolEvent = [this](CPhpFilePool *pool, tagSocketPoolEvent spe, CAsyncFile *handler) {
				SPA::CAutoLock al(this->m_cs);
				if (this->m_pe.isCallable()) {
					if (handler) {
						CPhpFile *h = new CPhpFile(pool, handler, false);
						this->m_pe((int)spe, Php::Object((SPA_CS_NS + PHP_FILE_HANDLER).c_str(), h), this);
					}
					else {
						this->m_pe((int)spe, nullptr, this);
					}
				}
			};
			m_pt = NotMS;
			break;
		default:
			if (m_defaultDb.size()) {
				std::wstring dfltDb = SPA::Utilities::ToWide(m_defaultDb.c_str(), m_defaultDb.size());
				if (slave) {
					Handler = new CMasterPool::CSlavePool(dfltDb.c_str(), recvTimeout, id);
					m_pt = Slave;
				}
				else {
					Handler = new CMasterPool(dfltDb.c_str(), recvTimeout, id);
					m_pt = Master;
				}
				Handler->SetConnTimeout(connTimeout);
				Handler->SetAutoConn(autoConn);
			}
			else {
				Handler = new CPhpPool(autoConn, recvTimeout, connTimeout, id);
				m_pt = NotMS;
			}
			Handler->DoSslServerAuthentication = [this](CPhpPool *pool, CClientSocket * cs)->bool {
				return this->DoSSLAuth(cs);
			};
			Handler->SocketPoolEvent = [this](CPhpPool *pool, tagSocketPoolEvent spe, CAsyncHandler *handler) {
				SPA::CAutoLock al(this->m_cs);
				if (this->m_pe.isCallable()) {
					if (handler) {
						CPhpHandler *h = new CPhpHandler(pool, handler, false);
						this->m_pe((int)spe, Php::Object((SPA_CS_NS + PHP_ASYNC_HANDLER).c_str(), h), this);
					}
					else {
						this->m_pe((int)spe, nullptr, this);
					}
				}
			};
			break;
		}
		if (slave) {
			m_defaultDb.clear();
		}
		m_nSvsId = id;
	}

	Php::Value CPhpSocketPool::Seek() {
		if (!Handler) {
			throw Php::Exception(NOT_INITIALIZED);
		}
		switch (m_nSvsId) {
		case SPA::Mysql::sidMysql:
		case SPA::Odbc::sidOdbc:
		case SPA::Sqlite::sidSqlite:
		{
			auto handler = Db->Seek();
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(Db, handler.get(), false));
			return obj;
		}
		break;
		case SPA::Queue::sidQueue:
		{
			auto handler = Queue->Seek();
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_QUEUE_HANDLER).c_str(), new CPhpQueue(Queue, handler.get(), false));
			return obj;
		}
		break;
		case SPA::SFile::sidFile:
		{
			auto handler = File->Seek();
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_FILE_HANDLER).c_str(), new CPhpFile(File, handler.get(), false));
			return obj;
		}
		break;
		default:
		{
			auto handler = Handler->Seek();
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_ASYNC_HANDLER).c_str(), new CPhpHandler(Handler, handler.get(), false));
			return obj;
		}
		break;
		}
		return nullptr; //shouldnot come here
	}

	Php::Value CPhpSocketPool::SeekByQueue(Php::Parameters &params) {
		std::string qName;
		if (params.size()) {
			qName = params[0].stringValue();
		}
		Trim(qName);
		bool empty = (qName.size() == 0);
		if (!Handler) {
			throw Php::Exception(NOT_INITIALIZED);
		}

		switch (m_nSvsId) {
		case SPA::Mysql::sidMysql:
		case SPA::Odbc::sidOdbc:
		case SPA::Sqlite::sidSqlite:
		{
			auto handler = empty ? Db->SeekByQueue() : Db->SeekByQueue(qName);
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(Db, handler.get(), false));
			return obj;
		}
		break;
		case SPA::Queue::sidQueue:
		{
			auto handler = empty ? Queue->SeekByQueue() : Queue->SeekByQueue(qName);
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_QUEUE_HANDLER).c_str(), new CPhpQueue(Queue, handler.get(), false));
			return obj;
		}
		break;
		case SPA::SFile::sidFile:
		{
			auto handler = empty ? File->SeekByQueue() : File->SeekByQueue(qName);
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_FILE_HANDLER).c_str(), new CPhpFile(File, handler.get(), false));
			return obj;
		}
		break;
		default:
		{
			auto handler = empty ? Handler->SeekByQueue() : Handler->SeekByQueue(qName);
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_ASYNC_HANDLER).c_str(), new CPhpHandler(Handler, handler.get(), false));
			return obj;
		}
		break;
		}
		return nullptr; //shouldnot come here
	}

	Php::Value CPhpSocketPool::Lock(Php::Parameters &params) {
		unsigned int timeout = (~0);
		if (params.size()) {
			timeout = (unsigned int)params[0].numericValue();
		}
		if (!Handler) {
			throw Php::Exception(NOT_INITIALIZED);
		}
		switch (m_nSvsId) {
		case SPA::Mysql::sidMysql:
		case SPA::Odbc::sidOdbc:
		case SPA::Sqlite::sidSqlite:
		{
			auto handler = Db->Lock(timeout);
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(Db, handler.get(), true));
			return obj;
		}
		break;
		case SPA::Queue::sidQueue:
		{
			auto handler = Queue->Lock(timeout);
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_QUEUE_HANDLER).c_str(), new CPhpQueue(Queue, handler.get(), true));
			return obj;
		}
		break;
		case SPA::SFile::sidFile:
		{
			auto handler = File->Lock(timeout);
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_FILE_HANDLER).c_str(), new CPhpFile(File, handler.get(), true));
			return obj;
		}
		break;
		default:
		{
			auto handler = Handler->Lock(timeout);
			if (!handler)
				return nullptr;
			Php::Object obj((SPA_CS_NS + PHP_ASYNC_HANDLER).c_str(), new CPhpHandler(Handler, handler.get(), true));
			return obj;
		}
		break;
		}
		return nullptr; //shouldnot come here
	}

	void CPhpSocketPool::ToCtx(const Php::Value &vCtx, SPA::ClientSide::CConnectionContext &ctx) {
		ToVariant(vCtx.get("AnyData"), ctx.AnyData);
		ctx.Host = vCtx.get("Host").stringValue();
		ctx.Port = (unsigned int)vCtx.get("Port").numericValue();
		ctx.EncrytionMethod = (SPA::tagEncryptionMethod)vCtx.get("EncrytionMethod").numericValue();
		std::string s = vCtx.get("UserId").stringValue();
		ctx.UserId = SPA::Utilities::ToWide(s.c_str(), s.size());
		s = vCtx.get("Password").stringValue();
		ctx.Password = SPA::Utilities::ToWide(s.c_str(), s.size());
		ctx.V6 = vCtx.get("V6").boolValue();
		ctx.Zip = vCtx.get("Zip").boolValue();
	}

	bool CPhpSocketPool::DoSSLAuth(CClientSocket *cs) {
		Php::Value ssl;
		SPA::IUcert *cert = cs->GetUCert();
		if (!cert) {
			return false;
		}
		{
			SPA::CAutoLock al(m_cs);
			if (!cert->Validity) {
				m_errCode = -1;
				m_errMsg = "Certificate not valid";
				return false;
			}
			m_errMsg = cert->Verify(&m_errCode);
			if (!m_errCode) {
				return true;
			}
			ssl = m_ssl;
		}
		if (ssl.isCallable()) {
			CPhpSocket *ps = new CPhpSocket(cs);
			return ssl(Php::Object((SPA_CS_NS + PHP_SOCKET).c_str(), ps), this).boolValue();
		}
		return false;
	}

	int CPhpSocketPool::__compare(const CPhpSocketPool &pool) const {
		if (!Handler || !pool.Handler) {
			return 1;
		}
		return (Handler == pool.Handler) ? 0 : 1;
	}

	Php::Value CPhpSocketPool::Start(Php::Parameters &params) {
		int64_t num;
		unsigned int socketsPerThread = 1, threads = 1;
		if (params.size() > 2) {
			num = params[2].numericValue();
			if (num > 1) {
				threads = (unsigned int)num;
			}
		}
		num = params[1].numericValue();
		if (num > 1) {
			socketsPerThread = (unsigned int)num;
		}
		if (!Handler) {
			throw Php::Exception(NOT_INITIALIZED);
		}
		if (Php::is_a(params[0], (SPA_CS_NS + PHP_CONN_CONTEXT).c_str())) {
			SPA::ClientSide::CConnectionContext ctx;
			ToCtx(params[0], ctx);
			switch (m_nSvsId) {
			case SPA::Mysql::sidMysql:
			case SPA::Odbc::sidOdbc:
			case SPA::Sqlite::sidSqlite:
				return Db->StartSocketPool(ctx, socketsPerThread, threads);
			case SPA::Queue::sidQueue:
				return Queue->StartSocketPool(ctx, socketsPerThread, threads);
			case SPA::SFile::sidFile:
				return File->StartSocketPool(ctx, socketsPerThread, threads);
			default:
				return Handler->StartSocketPool(ctx, socketsPerThread, threads);
			}
		}
		else if (params[0].isArray()) {

		}
		throw Php::Exception("One or an array of connection contexts are required");
	}

	Php::Value CPhpSocketPool::__get(const Php::Value &name) {
		int key = 0;
		if (!Handler) {
			throw Php::Exception(NOT_INITIALIZED);
		}
		else if (name == "Handlers" || name == "AsyncHandlers") {
			Php::Array harray;
			switch (m_nSvsId) {
			case SPA::Mysql::sidMysql:
			case SPA::Odbc::sidOdbc:
			case SPA::Sqlite::sidSqlite:
			{
				auto vH = Db->GetAsyncHandlers();
				for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
					CDBHandler *db = (*it).get();
					Php::Object objDb((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(Db, db, false));
					harray.set(key, objDb);
				}
			}
			break;
			case SPA::Queue::sidQueue:
			{
				auto vH = Queue->GetAsyncHandlers();
				for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
					CAsyncQueue *aq = (*it).get();
					Php::Object objQueue((SPA_CS_NS + PHP_QUEUE_HANDLER).c_str(), new CPhpQueue(Queue, aq, false));
					harray.set(key, objQueue);
				}
			}
			break;
			case SPA::SFile::sidFile:
			{
				auto vH = File->GetAsyncHandlers();
				for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
					CAsyncFile *af = (*it).get();
					Php::Object objFile((SPA_CS_NS + PHP_FILE_HANDLER).c_str(), new CPhpFile(File, af, false));
					harray.set(key, objFile);
				}
			}
			break;
			default:
			{
				auto vH = Handler->GetAsyncHandlers();
				for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
					CAsyncHandler *ah = (*it).get();
					Php::Object objHandler((SPA_CS_NS + PHP_ASYNC_HANDLER).c_str(), new CPhpHandler(Handler, ah, false));
					harray.set(key, objHandler);
				}
			}
				break;
			}
			return harray;
		}
		else if (name == "Sockets") {
			Php::Array harray;
			std::vector<std::shared_ptr<CClientSocket>> ss;
			switch (m_nSvsId) {
			case SPA::Mysql::sidMysql:
			case SPA::Odbc::sidOdbc:
			case SPA::Sqlite::sidSqlite:
				ss = Db->GetSockets();
				break;
			case SPA::sidChat:
				ss = Queue->GetSockets();
				break;
			case SPA::sidFile:
				ss = File->GetSockets();
				break;
			default:
				ss = Handler->GetSockets();
				break;
			}
			int key = 0;
			for (auto it = ss.cbegin(), end = ss.cend(); it != end; ++it, ++key) {
				CClientSocket *cs = it->get();
				Php::Object objSocket((SPA_CS_NS + PHP_SOCKET).c_str(), new CPhpSocket(cs));
				harray.set(key, objSocket);
			}
			return harray;
		}
		else if (name == "Conns" || name == "ConnectedSockets") {
			return (int64_t)Handler->GetConnectedSockets();
		}
		else if (name == "Disconns" || name == "DisconnectedSockets") {
			return (int64_t)Handler->GetDisconnectedSockets();
		}
		else if (name == "Idles" || name == "IdleSockets") {
			return (int64_t)Handler->GetIdleSockets();
		}
		else if (name == "Locks" || name == "LockedSockets") {
			return (int64_t)Handler->GetLockedSockets();
		}
		else if (name == "spt" || name == "SocketsPerThread") {
			return (int64_t)Handler->GetSocketsPerThread();
		}
		else if (name == "Threads" || name == "ThreadsCreated") {
			return (int64_t)Handler->GetThreadsCreated();
		}
		else if (name == "SvsId" || name == "ServiceId") {
			return (int64_t)m_nSvsId;
		}
		else if (name == "AutoConn") {
			return (int64_t)Handler->GetAutoConn();
		}
		else if (name == "RecvTimeout") {
			return (int64_t)Handler->GetRecvTimeout();
		}
		else if (name == "ConnTimeout") {
			return (int64_t)Handler->GetConnTimeout();
		}
		else if (name == "Queues") {
			return (int64_t)Handler->GetQueues();
		}
		else if (name == "id" || name == "PoolId") {
			return (int64_t)Handler->GetPoolId();
		}
		else if (name == "ec" || name == "ErrorCode") {
			SPA::CAutoLock al(m_cs);
			return m_errCode;
		}
		else if (name == "em" || name == "ErrorMsg") {
			SPA::CAutoLock al(m_cs);
			return m_errMsg;
		}
		else if (name == "Avg") {
			return Handler->IsAvg();
		}
		else if (name == "QueueName") {
			return Handler->GetQueueName();
		}
		else if (name == "AutoMerge" || name == "QueueAutoMerge") {
			return Handler->GetQueueAutoMerge();
		}
		else if (name == "Started") {
			return Handler->IsStarted();
		}
		else if (name == "Cache" && m_pt == Master) {
			switch (m_nSvsId) {
			case SPA::Mysql::sidMysql:
			case SPA::Odbc::sidOdbc:
			case SPA::Sqlite::sidSqlite:
			{
				CSQLMaster *master = (CSQLMaster *)Db;
			}
			break;
			case SPA::sidFile:
			case SPA::sidChat:
				assert(false); //shouldn't come here
				break;
			default:
			{
				CMasterPool *master = (CMasterPool *)Handler;
			}
			break;
			}
		}
		return Php::Base::__get(name);
	}

	void CPhpSocketPool::__set(const Php::Value &name, const Php::Value &value) {
		if (!Handler) {
			throw Php::Exception(NOT_INITIALIZED);
		}
		else if (name == "QueueName") {
			Handler->SetQueueName(value.stringValue().c_str());
		}
		else if (name == "AutoMerge" || name == "QueueAutoMerge") {
			Handler->SetQueueAutoMerge(value.boolValue());
		}
		else if (name == "Event" || name == "PoolEvent" || name == "SocketPoolEvent") {
			SPA::CAutoLock al(m_cs);
			if (value.isCallable()) {
				m_pe = value;
			}
			else if (value.isNull()) {
				m_pe = nullptr;
			}
			else {
				throw Php::Exception("A callback expected for socket pool event");
			}
		}
		else if (name == "VerifyCert") {
			SPA::CAutoLock al(m_cs);
			if (value.isCallable()) {
				m_ssl = value;
			}
			else if (value.isNull()) {
				m_ssl = nullptr;
			}
			else {
				throw Php::Exception("A callback expected to authenticate certificate from a remote server");
			}
		}
		else if (name == "AutoConn") {
			Handler->SetAutoConn(value.boolValue());
		}
		else if (name == "RecvTimeout") {
			Handler->SetRecvTimeout((unsigned int)value.numericValue());
		}
		else if (name == "ConnTimeout") {
			Handler->SetConnTimeout((unsigned int)value.numericValue());
		}
		else {
			Php::Base::__set(name, value);
		}
	}

	void CPhpSocketPool::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpSocketPool> pool(PHP_SOCKET_POOL);
		pool.property("DEFAULT_RECV_TIMEOUT", (int64_t)SPA::ClientSide::DEFAULT_RECV_TIMEOUT, Php::Const);
		pool.property("DEFAULT_CONN_TIMEOUT", (int64_t)SPA::ClientSide::DEFAULT_CONN_TIMEOUT, Php::Const);
		pool.method(PHP_CONSTRUCT, &CPhpSocketPool::__construct, {
			Php::ByVal("svsId", Php::Type::Numeric),
			Php::ByVal("defaultDb", Php::Type::String, false),
			Php::ByVal("autoConn", Php::Type::Bool, false),
			Php::ByVal("recvTimeout", Php::Type::Numeric, false),
			Php::ByVal("connTimeout", Php::Type::Numeric, false),
			Php::ByVal("slave", Php::Type::Bool, false)
		});
		pool.method("NewSlave", &CPhpSocketPool::NewSlave, {
			Php::ByVal("defaultDb", Php::Type::String, false),
			Php::ByVal("autoConn", Php::Type::Bool, false),
			Php::ByVal("recvTimeout", Php::Type::Numeric, false),
			Php::ByVal("connTimeout", Php::Type::Numeric, false)
		});
		pool.method("Shutdown", &CPhpSocketPool::ShutdownPool);
		pool.method("ShutdownPool", &CPhpSocketPool::ShutdownPool);
		pool.method("CloseAll", &CPhpSocketPool::DisconnectAll);
		pool.method("DisconnectAll", &CPhpSocketPool::DisconnectAll);
		pool.method("Lock", &CPhpSocketPool::Lock, {
			Php::ByVal("timeout", Php::Type::Numeric, false)
		});
		pool.method("Seek", &CPhpSocketPool::Seek);
		pool.method("SeekByQueue", &CPhpSocketPool::SeekByQueue, {
			Php::ByVal("qName", Php::Type::String, false)
		});
		pool.method("Start", &CPhpSocketPool::Start, {
			Php::ByVal("ctx_or_array", Php::Type::Null),
			Php::ByVal("socketsPerThread", Php::Type::Numeric),
			Php::ByVal("threads", Php::Type::Numeric, false),
		});
		pool.method("StartPool", &CPhpSocketPool::Start, {
			Php::ByVal("ctx_or_array", Php::Type::Null),
			Php::ByVal("socketsPerThread", Php::Type::Numeric),
			Php::ByVal("threads", Php::Type::Numeric, false),
		});
		pool.method("StartSocketPool", &CPhpSocketPool::Start, {
			Php::ByVal("ctx_or_array", Php::Type::Null),
			Php::ByVal("socketsPerThread", Php::Type::Numeric),
			Php::ByVal("threads", Php::Type::Numeric, false),
		});
		cs.add(std::move(pool));
	}
}
