#include "stdafx.h"
#include "PhpBuffer.h"

CPhpBuffer::CPhpBuffer(SPA::CUQueue *buffer) : m_pBuffer(buffer) {

}

CPhpBuffer::~CPhpBuffer() {
	SPA::CScopeUQueue::Unlock(m_pBuffer);
}

void CPhpBuffer::__construct(Php::Parameters &params) {
	if (m_pBuffer) {
		return;
	}
	unsigned int maxLen = SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE;
	unsigned int blockSize = SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE;
	if (params.size()) {
		auto len = params[0].numericValue();
		if (len < 0) {
			throw Php::Exception("Bad buffer size");
		}
		maxLen = (unsigned int)len;
	}
	if (params.size() > 1) {
		auto len = params[1].numericValue();
		if (len < 0) {
			throw Php::Exception("Bad buffer block size");
		}
		blockSize = (unsigned int)len;
	}
	m_pBuffer = SPA::CScopeUQueue::Lock(SPA::GetOS(), SPA::IsBigEndian(), maxLen, blockSize);
}

void CPhpBuffer::Empty() {
	if (m_pBuffer) {
		m_pBuffer->Empty();
	}
}

void CPhpBuffer::CleanTrack() {
	if (m_pBuffer) {
		m_pBuffer->CleanTrack();
	}
}

Php::Value CPhpBuffer::Discard(Php::Parameters &params) {
	if (!m_pBuffer) {
		return 0;
	}
	unsigned int bytes = (unsigned int)(params[0].numericValue());
	return (int64_t)m_pBuffer->Pop(bytes);
}

Php::Value CPhpBuffer::SaveDate(Php::Parameters &params) {
	if (!m_pBuffer) {
		m_pBuffer = SPA::CScopeUQueue::Lock();
	}
	Php::Value dt = params[0].call("format", "Y-m-d H:i:s.u");
	SPA::UDateTime udt(dt.rawValue());
	*m_pBuffer << udt.time;
	return this;
}

Php::Value CPhpBuffer::LoadDate() {
	if (!m_pBuffer) {
		m_pBuffer = SPA::CScopeUQueue::Lock();
	}
	SPA::UINT64 time;
	try {
		*m_pBuffer >> time;
		SPA::UDateTime udt(time);
		char str[64] = { 0 };
		udt.ToDBString(str, sizeof(str));
		return Php::Object("DateTime", str);
	}
	catch (SPA::CUException &ex) {
		auto message = ex.what();
		throw Php::Exception(message);
	}
	catch (...) {
		throw Php::Exception("Unknown error");
	}
}

void CPhpBuffer::RegisterInto(Php::Namespace &spa) {
	Php::Class<CPhpBuffer> buffer("CUQueue");
	buffer.property("DEFAULT_BUFFER_SIZE", (int64_t)SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE, Php::Const);
	buffer.property("DEFAULT_BLOCK_SIZE", (int64_t)SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE, Php::Const);
	buffer.method("__construct", &CPhpBuffer::__construct, {
		Php::ByVal("maxLen", Php::Type::Numeric, false),
		Php::ByVal("blockSize", Php::Type::Numeric, false)
	});
	buffer.method("Empty", &CPhpBuffer::Empty);
	buffer.method("CleanTrack", &CPhpBuffer::CleanTrack);
	buffer.method("Discard", &CPhpBuffer::Discard, {
		Php::ByVal("len", Php::Type::Numeric)
	});
	buffer.method("SaveDate", &CPhpBuffer::SaveDate, {
		Php::ByVal("dt", "DateTime", false, true)
	});
	buffer.method("LoadDate", &CPhpBuffer::LoadDate);
	spa.add(buffer);
}

Php::Value CPhpBuffer::__get(const Php::Value &name) {
	if (!m_pBuffer) {
		m_pBuffer = SPA::CScopeUQueue::Lock();
	}
	if (name == "Size") {
		return (int64_t)(m_pBuffer->GetSize());
	}
	else if (name == "HeadPosition") {
		return (int64_t)m_pBuffer->GetHeadPosition();
	}
	else if (name == "MaxBufferSize") {
		return (int64_t)m_pBuffer->GetMaxSize();
	}
	else if (name == "TailSize") {
		return (int64_t)m_pBuffer->GetTailSize();
	}
	else if (name == "OS") {
		return (int64_t)m_pBuffer->GetOS();
	}
	else if (name == "Endian") {
		return m_pBuffer->GetEndian();
	}
	return Php::Base::__get(name);
}

void CPhpBuffer::__set(const Php::Value &name, const Php::Value &value) {
	if (!m_pBuffer) {
		auto size = value.numericValue();
		if (name == "Size" && size > 0) {
			m_pBuffer = SPA::CScopeUQueue::Lock(SPA::GetOS(), SPA::IsBigEndian(), (unsigned int)size);
		}
		else {
			m_pBuffer = SPA::CScopeUQueue::Lock();
		}
	}
	if (name == "Size") {
		auto size = value.numericValue();
		if (size < 0) {
			size = 0;
		}
		if ((unsigned int)size > m_pBuffer->GetSize() + m_pBuffer->GetHeadPosition()) {
			throw Php::Exception("Invalid size value");
		}
		m_pBuffer->SetSize((unsigned int)size);
	}
	else if (name == "OS") {
		auto os = value.numericValue();
		if (os < SPA::osWin || os > SPA::osWinPhone) {
			throw Php::Exception("Bad operation system value");
		}
		m_pBuffer->SetOS((SPA::tagOperationSystem)os);
	}
	else if (name == "Endian") {
		auto endian = value.boolValue();
		m_pBuffer->SetEndian(endian);
	}
}
