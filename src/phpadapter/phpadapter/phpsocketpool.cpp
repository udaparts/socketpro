#include "stdafx.h"
#include "phpsocketpool.h"

namespace PA {

	CPhpSocketPool::CPhpSocketPool() : m_nSvsId(0), Handler(nullptr), m_errCode(0) {

	}

	CPhpSocketPool::~CPhpSocketPool() {

	}

	void CPhpSocketPool::__construct(Php::Parameters &params) {
		bool autoConn = true;
		unsigned int recvTimeout = SPA::ClientSide::DEFAULT_RECV_TIMEOUT;
		unsigned int connTimeout = SPA::ClientSide::DEFAULT_CONN_TIMEOUT;
		unsigned int nServiceId = (unsigned int)params[0].numericValue();
		size_t args = params.size();
		if (args > 4) {
			connTimeout = (unsigned int)params[4].numericValue();
		}
		if (args > 3) {
			recvTimeout = (unsigned int)params[4].numericValue();
		}
		if (args > 2) {
			autoConn = params[2].boolValue();
		}
		if (args > 1) {
			m_defaultDb = params[1].stringValue();
		}
		Trim(m_defaultDb);
	}

	Php::Value CPhpSocketPool::__get(const Php::Value &name) {
		if (!Handler) {
			return nullptr;
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
		if (!Handler) return;
		if (name == "QueueName") {
			Handler->SetQueueName(value.stringValue().c_str());
		}
		else if (name == "AutoMerge" || name == "QueueAutoMerge") {
			Handler->SetQueueAutoMerge(value.boolValue());
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
		Php::Class<CPhpSocketPool> buffer("CSocketPool");
		buffer.method("__construct", &CPhpSocketPool::__construct, {
			Php::ByVal("svsId", Php::Type::Numeric),
			Php::ByVal("defaultDb", Php::Type::String, false),
			Php::ByVal("autoConn", Php::Type::Bool, false),
			Php::ByVal("recvTimeout", Php::Type::Numeric, false),
			Php::ByVal("connTimeout", Php::Type::Numeric, false)
		});
		cs.add(std::move(buffer));
	}
}
