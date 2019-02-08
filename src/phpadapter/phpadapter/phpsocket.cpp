#include "stdafx.h"
#include "phpsocket.h"
#include "phpcert.h"

namespace PA {
	CPhpSocket::CPhpSocket(CClientSocket *cs) : m_cs(cs) {
	}

	CPhpSocket::~CPhpSocket() {
	}

	void CPhpSocket::__construct(Php::Parameters &params) {
	}

	int CPhpSocket::__compare(const CPhpSocket &socket) const {
		if (!m_cs || !socket.m_cs) {
			return 1;
		}
		return (m_cs == socket.m_cs) ? 0 : 1;
	}

	Php::Value CPhpSocket::__get(const Php::Value &name) {
		if (name == "Error") {
			auto em = m_cs->GetErrorMsg();
			Trim(em);
			Php::Value v;
			v.set(PHP_ERR_CODE, m_cs->GetErrorCode());
			v.set(PHP_ERR_MSG, em);
			return v;
		}
		else if (name == "Connected") {
			return m_cs->IsConnected();
		}
		else if (name == "ConnState" || name == "ConnectionState") {
			return m_cs->GetConnectionState();
		}
		else if (name == "Sendable") {
			return m_cs->Sendable();
		}
		else if (name == "Peer") {
			unsigned int port;
			bool endian;
			std::string s = m_cs->GetPeerName(&port);
			SPA::tagOperationSystem os = m_cs->GetPeerOs(&endian);
			Php::Value v;
			v.set("PeerName", s);
			v.set("OS", os);
			v.set("Port", (int64_t)port);
			v.set("Endian", endian);
			return v;
		}
		else if (name == "BytesInSendBuffer" || name == "BytesInSendingBuffer") {
			return (int64_t)m_cs->GetBytesInSendingBuffer();
		}
		else if (name == "BytesBatched") {
			return (int64_t)m_cs->GetBytesBatched();
		}
		else if (name == "BytesInRecvBuffer" || name == "BytesInReceivingBuffer") {
			return (int64_t)m_cs->GetBytesInReceivingBuffer();
		}
		else if (name == "EM" || name == "EncryptionMethod") {
			return m_cs->GetEncryptionMethod();
		}
		else if (name == "RequestsInQueue" || name == "CountOfRequestsInQueue") {
			return (int64_t)m_cs->GetCountOfRequestsInQueue();
		}
		else if (name == "Cert" || name == "UCert") {
			SPA::IUcert *cert = m_cs->GetUCert();
			if (!cert) {
				return nullptr;
			}
			return Php::Object((SPA_NS + PHP_CERT).c_str(), new CPhpCert(cert));
		}
		else if (name == "UID") {
			auto uid = m_cs->GetUID();
			return SPA::Utilities::ToUTF8(uid.c_str(), uid.size());
		}
		else if (name == "RecvTimeout" || name == "ReceivingTimeout") {
			return (int64_t)m_cs->GetRecvTimeout();
		}
		else if (name == "Zip") {
			return m_cs->GetZip();
		}
		else if (name == "ZipLevel") {
			return m_cs->GetZipLevel();
		}
		else if (name == "Random") {
			return m_cs->IsRandom();
		}
		else if (name == "Routing") {
			return m_cs->IsRouting();
		}
		else if (name == "PingTime" || name == "ServerPingTime") {
			return m_cs->GetServerPingTime();
		}
		else if (name == "ConnTimeout" || name == "ConnectingTimeout") {
			return (int64_t)m_cs->GetConnTimeout();
		}
		else if (name == "DequeuedMessageAborted") {
			return SPA::ClientSide::ClientCoreLoader.IsDequeuedMessageAborted(m_cs->GetHandle());
		}
		else if (name == "AutoConn") {
			return m_cs->GetAutoConn();
		}
		else if (name == "Batching") {
			return SPA::ClientSide::ClientCoreLoader.IsBatching(m_cs->GetHandle());
		}
		else if (name == "BytesReceived") {
			return (int64_t)m_cs->GetBytesReceived();
		}
		else if (name == "BytesSent") {
			return (int64_t)m_cs->GetBytesSent();
		}
		return Php::Base::__get(name);
	}

	void CPhpSocket::__set(const Php::Value &name, const Php::Value &value) {
		if (name == "Zip") {
			m_cs->SetZip(value.boolValue());
		}
		else if (name == "ZipLevel") {
			auto zl = value.numericValue();
			if (zl < 0 || zl > SPA::zlBestCompression) {
				throw Php::Exception("Bad compression value");
			}
			m_cs->SetZipLevel((SPA::tagZipLevel)zl);
		}
		else if (name == "RecvTimeout" || name == "ReceivingTimeout") {
			return m_cs->SetRecvTimeout((unsigned int)value.numericValue());
		}
		else if (name == "ConnTimeout" || name == "ConnectingTimeout") {
			m_cs->SetConnTimeout((unsigned int)value.numericValue());
		}
		Php::Base::__set(name, value);
	}

	void CPhpSocket::AbortDequeuedMessage() {
		SPA::ClientSide::ClientCoreLoader.AbortDequeuedMessage(m_cs->GetHandle());
	}

	Php::Value CPhpSocket::Cancel(Php::Parameters &params) {
		unsigned int cancel = (~0);
		if (params.size()) {
			cancel = (unsigned int)params[0].numericValue();
		}
		return m_cs->Cancel(cancel);
	}

	Php::Value CPhpSocket::DoEcho() {
		return m_cs->DoEcho();
	}

	void CPhpSocket::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpSocket> socket(PHP_SOCKET);
		socket.property("Version", SPA::ClientSide::ClientCoreLoader.GetUClientSocketVersion(), Php::Const);
		socket.method(PHP_CONSTRUCT, &CPhpSocket::__construct, Php::Private);
		socket.method("DoEcho", &CPhpSocket::DoEcho);
		socket.method("Cancel", &CPhpSocket::Cancel, {
			Php::ByVal("cancel", Php::Type::Numeric, false)
		});
		socket.method("SetZipLevelAtSvr", &CPhpSocket::SetZipLevelAtSvr, {
			Php::ByVal("zl", Php::Type::Numeric)
		});
		socket.method("TurnOnZipAtSvr", &CPhpSocket::TurnOnZipAtSvr, {
			Php::ByVal("zip", Php::Type::Bool, false)
		});
		socket.method("AbortDequeuedMessage", &CPhpSocket::AbortDequeuedMessage);
		cs.add(socket);
	}

	Php::Value CPhpSocket::TurnOnZipAtSvr(Php::Parameters &params) {
		bool zip = true;
		if (params[0].size()) {
			zip = params[0].boolValue();
		}
		return m_cs->TurnOnZipAtSvr(zip);
	}

	Php::Value CPhpSocket::SetZipLevelAtSvr(Php::Parameters &params) {
		auto zl = params[0].numericValue();
		if (zl < 0 || zl > SPA::zlBestCompression) {
			throw Php::Exception("Bad compression value");
		}
		return m_cs->SetZipLevelAtSvr((SPA::tagZipLevel)zl);
	}
} //namespace PA