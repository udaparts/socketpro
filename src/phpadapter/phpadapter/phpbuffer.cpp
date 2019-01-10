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

void CPhpBuffer::EnsureBuffer() {
	if (!m_pBuffer) {
		m_pBuffer = SPA::CScopeUQueue::Lock();
	}
}

#define BufferLoadCatch catch(SPA::CUException&ex){auto msg=ex.what();throw Php::Exception(msg);}catch (std::exception &ex) {auto msg=ex.what();throw Php::Exception(msg);}catch(...){throw Php::Exception("Unknown exception");}

Php::Value CPhpBuffer::SaveDate(Php::Parameters &params) {
	EnsureBuffer();
	Php::Value dt = params[0].call("format", "Y-m-d H:i:s.u");
	SPA::UDateTime udt(dt.rawValue());
	*m_pBuffer << udt.time;
	return this;
}

Php::Value CPhpBuffer::LoadDate() {
	EnsureBuffer();
	try {
		SPA::UINT64 time;
		*m_pBuffer >> time;
		SPA::UDateTime udt(time);
		char str[64] = { 0 };
		udt.ToDBString(str, sizeof(str));
		return Php::Object("DateTime", str);
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveByte(Php::Parameters &params) {
	auto data = params[0].numericValue();
	if (data < 0 || data > 0xff) {
		throw Php::Exception("Invalid byte value");
	}
	unsigned char b = (unsigned char)data;
	EnsureBuffer();
	m_pBuffer->Push(&b, sizeof(b));
	return this;
}

Php::Value CPhpBuffer::LoadByte() {
	EnsureBuffer();
	try {
		unsigned char b;
		m_pBuffer->Pop(&b, sizeof(b));
		return b;
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveBool(Php::Parameters &params) {
	bool b = params[0].boolValue();
	EnsureBuffer();
	m_pBuffer->Push((const unsigned char*)&b, sizeof(b));
	return this;
}

Php::Value CPhpBuffer::LoadBool() {
	EnsureBuffer();
	try {
		unsigned char b;
		m_pBuffer->Pop(&b, sizeof(b));
		return (b != 0);
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveAChar(Php::Parameters &params) {
	auto data = params[0].numericValue();
	if (data < -0x7f || data > 0x7f) {
		throw Php::Exception("Invalid char value");
	}
	char b = (char)data;
	EnsureBuffer();
	m_pBuffer->Push((const unsigned char*)&b, sizeof(b));
	return this;
}

Php::Value CPhpBuffer::LoadAChar() {
	EnsureBuffer();
	try {
		char b;
		m_pBuffer->Pop((unsigned char*)&b, sizeof(b));
		return b;
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveShort(Php::Parameters &params) {
	auto data = params[0].numericValue();
	if (data < -0x7fff || data > 0x7fff) {
		throw Php::Exception("Invalid short value");
	}
	short b = (short)data;
	EnsureBuffer();
	m_pBuffer->Push((const unsigned char*)&b, sizeof(b));
	return this;
}

Php::Value CPhpBuffer::LoadShort() {
	EnsureBuffer();
	try {
		short b;
		m_pBuffer->Pop((unsigned char*)&b, sizeof(b));
		return b;
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveInt(Php::Parameters &params) {
	auto data = params[0].numericValue();
	if (data < -0x7fffffff || data > 0x7fffffff) {
		throw Php::Exception("Invalid int value");
	}
	int b = (int)data;
	EnsureBuffer();
	m_pBuffer->Push((const unsigned char*)&b, sizeof(b));
	return this;
}

Php::Value CPhpBuffer::LoadInt() {
	EnsureBuffer();
	try {
		int b;
		m_pBuffer->Pop((unsigned char*)&b, sizeof(b));
		return b;
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveUInt(Php::Parameters &params) {
	auto data = params[0].numericValue();
	if (data < 0 ||data > 0xffffffff) {
		throw Php::Exception("Invalid unsigned int value");
	}
	unsigned int b = (unsigned int)data;
	EnsureBuffer();
	m_pBuffer->Push((const unsigned char*)&b, sizeof(b));
	return this;
}

Php::Value CPhpBuffer::LoadUInt() {
	EnsureBuffer();
	try {
		unsigned int b;
		m_pBuffer->Pop((unsigned char*)&b, sizeof(b));
		return (int64_t)b;
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveUShort(Php::Parameters &params) {
	auto data = params[0].numericValue();
	if (data < 0 || data > 0xffff) {
		throw Php::Exception("Invalid unsigned short value");
	}
	unsigned short b = (unsigned short)data;
	EnsureBuffer();
	m_pBuffer->Push((const unsigned char*)&b, sizeof(b));
	return this;
}

Php::Value CPhpBuffer::LoadUShort() {
	EnsureBuffer();
	try {
		unsigned short b;
		m_pBuffer->Pop((unsigned char*)&b, sizeof(b));
		return b;
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveLong(Php::Parameters &params) {
	auto data = params[0].numericValue();
	EnsureBuffer();
	m_pBuffer->Push((const unsigned char*)&data, sizeof(data));
	return this;
}

Php::Value CPhpBuffer::LoadLong() {
	EnsureBuffer();
	try {
		int64_t b;
		m_pBuffer->Pop((unsigned char*)&b, sizeof(b));
		return b;
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveDouble(Php::Parameters &params) {
	auto data = params[0].floatValue();
	EnsureBuffer();
	m_pBuffer->Push((const unsigned char*)&data, sizeof(data));
	return this;
}

Php::Value CPhpBuffer::LoadDouble() {
	EnsureBuffer();
	try {
		double b;
		m_pBuffer->Pop((unsigned char*)&b, sizeof(b));
		return b;
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveFloat(Php::Parameters &params) {
	auto data = params[0].floatValue();
	float f = (float)data;
	EnsureBuffer();
	m_pBuffer->Push((const unsigned char*)&f, sizeof(f));
	return this;
}

Php::Value CPhpBuffer::LoadFloat() {
	EnsureBuffer();
	try {
		float b;
		m_pBuffer->Pop((unsigned char*)&b, sizeof(b));
		return b;
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveAString(Php::Parameters &params) {
	auto data = params[0].rawValue();
	EnsureBuffer();
	*m_pBuffer << data;
	return this;
}

Php::Value CPhpBuffer::LoadAString() {
	EnsureBuffer();
	try {
		unsigned int *len = (unsigned int*) m_pBuffer->GetBuffer();
		if (*len == (~0)) {
			//deal with nullptr string
			unsigned int n;
			*m_pBuffer >> n;
			return nullptr;
		}
		std::string s;
		*m_pBuffer >> s;
		return s.c_str();
	}
	BufferLoadCatch
}

Php::Value CPhpBuffer::SaveString(Php::Parameters &params) {
	auto data = params[0].rawValue();
	EnsureBuffer();
	if (data) {
		SPA::CScopeUQueue sp;
		SPA::Utilities::ToWide(data, ::strlen(data), *sp);
		m_pBuffer->Push(sp->GetBuffer(), sp->GetSize());
	}
	else {
		//nullptr string
		*m_pBuffer << data;
	}
	return this;
}

Php::Value CPhpBuffer::LoadAString() {
	EnsureBuffer();
	try {
		unsigned int *len = (unsigned int*)m_pBuffer->GetBuffer();
		if (*len == (~0)) {
			//deal with nullptr string
			unsigned int n;
			*m_pBuffer >> n;
			return nullptr;
		}
		std::string s;
		*m_pBuffer >> s;
		return s.c_str();
	}
	BufferLoadCatch
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
	buffer.method("SaveByte", &CPhpBuffer::SaveByte, {
		Php::ByVal("b", Php::Type::Numeric)
	});
	buffer.method("LoadByte", &CPhpBuffer::LoadByte);
	buffer.method("SaveAChar", &CPhpBuffer::SaveAChar, {
		Php::ByVal("c", Php::Type::Numeric)
	});
	buffer.method("LoadAChar", &CPhpBuffer::LoadAChar);
	buffer.method("SaveBool", &CPhpBuffer::SaveBool, {
		Php::ByVal("b")
	});
	buffer.method("LoadBool", &CPhpBuffer::LoadBool);
	buffer.method("SaveShort", &CPhpBuffer::SaveShort, {
		Php::ByVal("s", Php::Type::Numeric)
	});
	buffer.method("LoadShort", &CPhpBuffer::LoadShort);
	buffer.method("SaveInt", &CPhpBuffer::SaveInt, {
		Php::ByVal("i", Php::Type::Numeric)
	});
	buffer.method("LoadInt", &CPhpBuffer::LoadInt);
	buffer.method("SaveUInt", &CPhpBuffer::SaveUInt, {
		Php::ByVal("ui", Php::Type::Numeric)
	});
	buffer.method("LoadUInt", &CPhpBuffer::LoadUInt);
	buffer.method("SaveUShort", &CPhpBuffer::SaveUShort, {
		Php::ByVal("us", Php::Type::Numeric)
	});
	buffer.method("LoadUShort", &CPhpBuffer::LoadUShort);
	buffer.method("SaveLong", &CPhpBuffer::SaveLong, {
		Php::ByVal("l", Php::Type::Numeric)
	});
	buffer.method("LoadLong", &CPhpBuffer::LoadLong);
	buffer.method("SaveULong", &CPhpBuffer::SaveLong, {
		Php::ByVal("ul", Php::Type::Numeric)
	});
	buffer.method("LoadULong", &CPhpBuffer::LoadLong);
	buffer.method("SaveChar", &CPhpBuffer::SaveUShort, {
		Php::ByVal("wc", Php::Type::Numeric)
	});
	buffer.method("LoadChar", &CPhpBuffer::LoadUShort);
	buffer.method("SaveDouble", &CPhpBuffer::SaveDouble, {
		Php::ByVal("d", Php::Type::Float)
	});
	buffer.method("LoadDouble", &CPhpBuffer::LoadDouble);
	buffer.method("SaveFloat", &CPhpBuffer::SaveFloat, {
		Php::ByVal("f", Php::Type::Float)
	});
	buffer.method("LoadFloat", &CPhpBuffer::LoadFloat);
	buffer.method("SaveAString", &CPhpBuffer::SaveAString, {
		Php::ByVal("as", Php::Type::String) //ASCII string
	});
	buffer.method("LoadAString", &CPhpBuffer::LoadAString);
	buffer.method("SaveString", &CPhpBuffer::SaveString, {
		Php::ByVal("ws", Php::Type::String) //UNICODE string
	});
	buffer.method("LoadString", &CPhpBuffer::LoadString);

	spa.add(buffer);
}

Php::Value CPhpBuffer::__get(const Php::Value &name) {
	EnsureBuffer();
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
	else {
		Php::Base::__set(name, value);
	}
}
