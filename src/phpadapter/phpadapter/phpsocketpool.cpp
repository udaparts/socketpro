#include "stdafx.h"
#include "phpsocketpool.h"

namespace PA {

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
		if (m_pt != Master) {
			throw Php::Exception("Cannot create a slave pool from a non-master pool");
		}
		if (!Handler) {
			throw Php::Exception("Root master pool not available");
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
		return Php::Object(PHP_SOCKET_POOL, (int64_t)m_nSvsId, defaultDb, autoConn, (int64_t)recvTimeout, (int64_t)connTimeout, true);
	}

	void CPhpSocketPool::ShutdownPool(Php::Parameters &params) {
		if (Handler) {
			Handler->ShutdownPool();
		}
	}

	Php::Value CPhpSocketPool::DisconnectAll(Php::Parameters &params) {
		bool ok = true;
		if (Handler) {
			ok = Handler->DisconnectAll();
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
				return true;
			};
			Db->SocketPoolEvent = [this](CPhpDbPool *pool, SPA::ClientSide::tagSocketPoolEvent spe, CDBHandler *handler) {
				SPA::CAutoLock al(this->m_cs);
				if (this->m_pe.isCallable()) {

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
				return true;
			};
			Queue->SocketPoolEvent = [this](CPhpQueuePool *pool, SPA::ClientSide::tagSocketPoolEvent spe, CAsyncQueue *handler) {
				SPA::CAutoLock al(this->m_cs);
				if (this->m_pe.isCallable()) {

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
				return true;
			};
			File->SocketPoolEvent = [this](CPhpFilePool *pool, SPA::ClientSide::tagSocketPoolEvent spe, CAsyncFile *handler) {
				SPA::CAutoLock al(this->m_cs);
				if (this->m_pe.isCallable()) {

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
				return true;
			};
			Handler->SocketPoolEvent = [this](CPhpPool *pool, SPA::ClientSide::tagSocketPoolEvent spe, CAsyncHandler *handler) {
				SPA::CAutoLock al(this->m_cs);
				if (this->m_pe.isCallable()) {

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
		SPA::CAutoLock al(m_cs);
		if (!Handler) {
			return nullptr;
		}
		switch (m_nSvsId) {
		case SPA::Mysql::sidMysql:
		case SPA::Odbc::sidOdbc:
		case SPA::Sqlite::sidSqlite:
		{
			auto handler = Db->Seek();
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_DB_HANDLER, new CPhpDb(Db, handler.get(), false));
			return obj;
		}
		break;
		case SPA::Queue::sidQueue:
		{
			auto handler = Queue->Seek();
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_QUEUE_HANDLER, new CPhpQueue(Queue, handler.get(), false));
			return obj;
		}
		break;
		case SPA::SFile::sidFile:
		{
			auto handler = File->Seek();
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_FILE_HANDLER, new CPhpFile(File, handler.get(), false));
			return obj;
		}
		break;
		default:
		{
			auto handler = Handler->Seek();
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_ASYNC_HANDLER, new CPhpHandler(Handler, handler.get(), false));
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
		SPA::CAutoLock al(m_cs);
		if (!Handler) {
			return nullptr;
		}

		switch (m_nSvsId) {
		case SPA::Mysql::sidMysql:
		case SPA::Odbc::sidOdbc:
		case SPA::Sqlite::sidSqlite:
		{
			auto handler = empty ? Db->SeekByQueue() : Db->SeekByQueue(qName);
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_DB_HANDLER, new CPhpDb(Db, handler.get(), false));
			return obj;
		}
		break;
		case SPA::Queue::sidQueue:
		{
			auto handler = empty ? Queue->SeekByQueue() : Queue->SeekByQueue(qName);
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_QUEUE_HANDLER, new CPhpQueue(Queue, handler.get(), false));
			return obj;
		}
		break;
		case SPA::SFile::sidFile:
		{
			auto handler = empty ? File->SeekByQueue() : File->SeekByQueue(qName);
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_FILE_HANDLER, new CPhpFile(File, handler.get(), false));
			return obj;
		}
		break;
		default:
		{
			auto handler = empty ? Handler->SeekByQueue() : Handler->SeekByQueue(qName);
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_ASYNC_HANDLER, new CPhpHandler(Handler, handler.get(), false));
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
		SPA::CAutoLock al(m_cs);
		if (!Handler) {
			return nullptr;
		}
		switch (m_nSvsId) {
		case SPA::Mysql::sidMysql:
		case SPA::Odbc::sidOdbc:
		case SPA::Sqlite::sidSqlite:
		{
			auto handler = Db->Lock(timeout);
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_DB_HANDLER, new CPhpDb(Db, handler.get(), true));
			return obj;
		}
		break;
		case SPA::Queue::sidQueue:
		{
			auto handler = Queue->Lock(timeout);
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_QUEUE_HANDLER, new CPhpQueue(Queue, handler.get(), true));
			return obj;
		}
		break;
		case SPA::SFile::sidFile:
		{
			auto handler = File->Lock(timeout);
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_FILE_HANDLER, new CPhpFile(File, handler.get(), true));
			return obj;
		}
		break;
		default:
		{
			auto handler = Handler->Lock(timeout);
			if (!handler)
				return nullptr;
			Php::Object obj(PHP_ASYNC_HANDLER, new CPhpHandler(Handler, handler.get(), true));
			return obj;
		}
		break;
		}
		return nullptr; //shouldnot come here
	}

	Php::Value CPhpSocketPool::Start(Php::Parameters &params) {
		return nullptr;
	}

	Php::Value CPhpSocketPool::__get(const Php::Value &name) {
		int key = 0;
		if (!Handler) {
			return nullptr;
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
					Php::Object objDb(PHP_DB_HANDLER, new CPhpDb(Db, db, false));
					harray.set(key, objDb);
				}
			}
			break;
			case SPA::Queue::sidQueue:
			{
				auto vH = Queue->GetAsyncHandlers();
				for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
					CAsyncQueue *aq = (*it).get();
					Php::Object objQueue(PHP_QUEUE_HANDLER, new CPhpQueue(Queue, aq, false));
					harray.set(key, objQueue);
				}
			}
			break;
			case SPA::SFile::sidFile:
			{
				auto vH = File->GetAsyncHandlers();
				for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
					CAsyncFile *af = (*it).get();
					Php::Object objFile(PHP_FILE_HANDLER, new CPhpFile(File, af, false));
					harray.set(key, objFile);
				}
			}
			break;
			default:
			{
				auto vH = Handler->GetAsyncHandlers();
				for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
					CAsyncHandler *ah = (*it).get();
					Php::Object objHandler(PHP_ASYNC_HANDLER, new CPhpHandler(Handler, ah, false));
					harray.set(key, objHandler);
				}
			}
				break;
			}
			return harray;
		}
		else if (name == "Sockets") {
			return (int64_t)Handler->GetConnectedSockets();
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
		return Php::Base::__get(name);
	}

	void CPhpSocketPool::__set(const Php::Value &name, const Php::Value &value) {
		if (!Handler) {
			return;
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
		cs.add(std::move(pool));
	}
}
