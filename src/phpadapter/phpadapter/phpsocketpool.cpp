#include "stdafx.h"
#include "phpsocketpool.h"
#include "phpsocket.h"
#include "phpconncontext.h"

namespace PA {

	const char* CPhpSocketPool::NOT_INITIALIZED = "Socket pool object not initialized";

	CPhpSocketPool::CPhpSocketPool(const CPoolStartContext &psc)
		: m_nSvsId(psc.SvsId), Handler(psc.PhpHandler), m_pt(psc.PoolType) {
	}

	CPhpSocketPool::~CPhpSocketPool() {

	}

	void CPhpSocketPool::__construct(Php::Parameters &params) {
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

	int CPhpSocketPool::__compare(const CPhpSocketPool &pool) const {
		if (!Handler || !pool.Handler) {
			return 1;
		}
		return (Handler == pool.Handler) ? 0 : 1;
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
		else if (name == KEY_THREADS || name == "ThreadsCreated") {
			return (int64_t)Handler->GetThreadsCreated();
		}
		else if (name == KEY_SVS_ID || name == "ServiceId") {
			return (int64_t)m_nSvsId;
		}
		else if (name == KEY_AUTO_CONN) {
			return Handler->GetAutoConn();
		}
		else if (name == KEY_RECV_TIMEOUT) {
			return (int64_t)Handler->GetRecvTimeout();
		}
		else if (name == KEY_CONN_TIMEOUT) {
			return (int64_t)Handler->GetConnTimeout();
		}
		else if (name == "Queues") {
			return (int64_t)Handler->GetQueues();
		}
		else if (name == "id" || name == "PoolId") {
			return (int64_t)Handler->GetPoolId();
		}
		else if (name == "Avg") {
			return Handler->IsAvg();
		}
		else if (name == "QueueName") {
			return Handler->GetQueueName();
		}
		else if (name == KEY_AUTO_MERGE || name == "QueueAutoMerge") {
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
		else if (name == KEY_AUTO_MERGE || name == "QueueAutoMerge") {
			Handler->SetQueueAutoMerge(value.boolValue());
		}
		else if (name == KEY_AUTO_CONN) {
			Handler->SetAutoConn(value.boolValue());
		}
		else if (name == KEY_RECV_TIMEOUT) {
			Handler->SetRecvTimeout((unsigned int)value.numericValue());
		}
		else if (name == KEY_CONN_TIMEOUT) {
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
		pool.method(PHP_CONSTRUCT, &CPhpSocketPool::__construct, Php::Private);
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
