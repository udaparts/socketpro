#include "stdafx.h"
#include "phpbuffer.h"

namespace PA
{

    CPhpBuffer::CPhpBuffer() : m_pBuffer(nullptr), m_bRelease(true) {
    }

    CPhpBuffer::CPhpBuffer(SPA::CUQueue * buff) : m_pBuffer(buff), m_bRelease(buff ? false : true) {
    }

    CPhpBuffer::~CPhpBuffer() {
        if (m_bRelease) {
            SPA::CScopeUQueue::Unlock(m_pBuffer);
        }
    }

    void CPhpBuffer::__destruct() {
    }

    void CPhpBuffer::Swap(CPhpBuffer * qPhp) {
        if (qPhp && qPhp->m_pBuffer) {
            EnsureBuffer();
            qPhp->m_pBuffer->Swap(*m_pBuffer);
        }
    }

    void CPhpBuffer::Swap(SPA::CUQueue * q) {
        if (q) {
            EnsureBuffer();
            q->Swap(*m_pBuffer);
        }
    }

    int CPhpBuffer::__compare(const CPhpBuffer & b) const {
        if (!m_pBuffer || !b.m_pBuffer) {
            return 1;
        }
        return (m_pBuffer == b.m_pBuffer) ? 0 : 1;
    }

    void CPhpBuffer::__construct(Php::Parameters & params) {
        if (m_pBuffer) {
            return;
        }
        unsigned int maxLen = SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE;
        unsigned int blockSize = SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE;
        if (params.size() && params[0].isNumeric()) {
            auto len = params[0].numericValue();
            if (len < 0) {
                throw Php::Exception("Bad buffer size");
            }
            maxLen = (unsigned int) len;
        }
        if (params.size() > 1 && params[1].isNumeric()) {
            auto len = params[1].numericValue();
            if (len < 0) {
                throw Php::Exception("Bad buffer block size");
            }
            blockSize = (unsigned int) len;
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

    Php::Value CPhpBuffer::Discard(Php::Parameters & params) {
        if (!m_pBuffer) {
            return 0;
        }
        unsigned int bytes = (unsigned int) (params[0].numericValue());
        return (int64_t) m_pBuffer->Pop(bytes);
    }

    void CPhpBuffer::EnsureBuffer(SPA::CUQueue *q, bool auto_release) {
        if (q) {
            if (m_bRelease) {
                SPA::CScopeUQueue::Unlock(m_pBuffer);
            }
            m_pBuffer = q;
            m_bRelease = auto_release;
        } else if (!m_pBuffer) {
            m_pBuffer = SPA::CScopeUQueue::Lock();
            m_bRelease = true;
        }
    }

    SPA::CUQueue * CPhpBuffer::GetBuffer() {
        if (!m_pBuffer) {
            m_pBuffer = SPA::CScopeUQueue::Lock();
        }
        return m_pBuffer;
    }

#define BufferLoadCatch catch(SPA::CUException&ex){auto msg=ex.what();throw Php::Exception(msg);}catch(std::exception &ex){auto msg=ex.what();throw Php::Exception(msg);}catch(...){throw Php::Exception("Unknown exception");}

    Php::Value CPhpBuffer::SaveDate(Php::Parameters & params) {
        EnsureBuffer();
        SaveDate(params[0]);
        return this;
    }

    void CPhpBuffer::SaveDate(const Php::Value & param) {
        Php::Value dt = param.call("format", "Y-m-d H:i:s.u");
        SPA::UDateTime udt(dt.rawValue());
        *m_pBuffer << udt.time;
    }

    Php::Value CPhpBuffer::LoadDate() {
        EnsureBuffer();
        try{
            SPA::UINT64 time;
            *m_pBuffer >> time;
            SPA::UDateTime udt(time);
            char str[64] =
            { 0};
            udt.ToDBString(str, sizeof (str));
            return Php::Object("DateTime", str);
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveByte(Php::Parameters & params) {
        auto data = params[0].numericValue();
        if (data < 0 || data > 0xff) {
            throw Php::Exception("Invalid byte value");
        }
        unsigned char b = (unsigned char) data;
        EnsureBuffer();
        m_pBuffer->Push(&b, sizeof (b));
        return this;
    }

    Php::Value CPhpBuffer::LoadByte() {
        EnsureBuffer();
        try{
            unsigned char b;
            m_pBuffer->Pop(&b, sizeof (b));
            return b;
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveBool(Php::Parameters & params) {
        bool b = params[0].boolValue();
        EnsureBuffer();
        m_pBuffer->Push((const unsigned char*) &b, sizeof (b));
        return this;
    }

    Php::Value CPhpBuffer::LoadBool() {
        EnsureBuffer();
        try{
            unsigned char b;
            m_pBuffer->Pop(&b, sizeof (b));
            return (b != 0);
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveAChar(Php::Parameters & params) {
        auto data = params[0].numericValue();
        if (data < -0x7f || data > 0x7f) {
            throw Php::Exception("Invalid char value");
        }
        char b = (char) data;
        EnsureBuffer();
        m_pBuffer->Push((const unsigned char*) &b, sizeof (b));
        return this;
    }

    Php::Value CPhpBuffer::LoadAChar() {
        EnsureBuffer();
        try{
            char b;
            m_pBuffer->Pop((unsigned char*) &b, sizeof (b));
            return b;
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveShort(Php::Parameters & params) {
        auto data = params[0].numericValue();
        if (data < -0x7fff || data > 0x7fff) {
            throw Php::Exception("Invalid short value");
        }
        short b = (short) data;
        EnsureBuffer();
        m_pBuffer->Push((const unsigned char*) &b, sizeof (b));
        return this;
    }

    Php::Value CPhpBuffer::LoadShort() {
        EnsureBuffer();
        try{
            short b;
            m_pBuffer->Pop((unsigned char*) &b, sizeof (b));
            return b;
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveInt(Php::Parameters & params) {
        auto data = params[0].numericValue();
        if (data < -0x7fffffff || data > 0x7fffffff) {
            throw Php::Exception("Invalid int value");
        }
        int b = (int) data;
        EnsureBuffer();
        m_pBuffer->Push((const unsigned char*) &b, sizeof (b));
        return this;
    }

    Php::Value CPhpBuffer::LoadInt() {
        EnsureBuffer();
        try{
            int b;
            m_pBuffer->Pop((unsigned char*) &b, sizeof (b));
            return b;
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveUInt(Php::Parameters & params) {
        auto data = params[0].numericValue();
        if (data < 0 || data > 0xffffffff) {
            throw Php::Exception("Invalid unsigned int value");
        }
        unsigned int b = (unsigned int) data;
        EnsureBuffer();
        m_pBuffer->Push((const unsigned char*) &b, sizeof (b));
        return this;
    }

    Php::Value CPhpBuffer::LoadUInt() {
        EnsureBuffer();
        try{
            unsigned int b;
            m_pBuffer->Pop((unsigned char*) &b, sizeof (b));
            return (int64_t) b;
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveUShort(Php::Parameters & params) {
        auto data = params[0].numericValue();
        if (data < 0 || data > 0xffff) {
            throw Php::Exception("Invalid unsigned short value");
        }
        unsigned short b = (unsigned short) data;
        EnsureBuffer();
        m_pBuffer->Push((const unsigned char*) &b, sizeof (b));
        return this;
    }

    Php::Value CPhpBuffer::LoadUShort() {
        EnsureBuffer();
        try{
            unsigned short b;
            m_pBuffer->Pop((unsigned char*) &b, sizeof (b));
            return b;
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveLong(Php::Parameters & params) {
        auto data = params[0].numericValue();
        EnsureBuffer();
        m_pBuffer->Push((const unsigned char*) &data, sizeof (data));
        return this;
    }

    Php::Value CPhpBuffer::LoadLong() {
        EnsureBuffer();
        try{
            int64_t b;
            m_pBuffer->Pop((unsigned char*) &b, sizeof (b));
            return b;
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveDouble(Php::Parameters & params) {
        auto data = params[0].floatValue();
        EnsureBuffer();
        m_pBuffer->Push((const unsigned char*) &data, sizeof (data));
        return this;
    }

    Php::Value CPhpBuffer::LoadDouble() {
        EnsureBuffer();
        try{
            double b;
            m_pBuffer->Pop((unsigned char*) &b, sizeof (b));
            return b;
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveFloat(Php::Parameters & params) {
        auto data = params[0].floatValue();
        float f = (float) data;
        EnsureBuffer();
        m_pBuffer->Push((const unsigned char*) &f, sizeof (f));
        return this;
    }

    Php::Value CPhpBuffer::LoadFloat() {
        EnsureBuffer();
        try{
            float b;
            m_pBuffer->Pop((unsigned char*) &b, sizeof (b));
            return b;
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveAString(Php::Parameters & params) {
        EnsureBuffer();
        SaveAString(params[0]);
        return this;
    }

    Php::Value CPhpBuffer::LoadAString() {
        EnsureBuffer();
        try{
            unsigned int *len = (unsigned int*) m_pBuffer->GetBuffer();
            if (*len == (~0)) {
                //deal with nullptr string
                unsigned int n;
                *m_pBuffer >> n;
                return nullptr;
            }
            std::string s;
            *m_pBuffer >> s;
            return s;
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::SaveDecimal(Php::Parameters & params) {
        EnsureBuffer();
        SaveDecimal(params[0]);
        return this;
    }

    void CPhpBuffer::SaveDecimal(const Php::Value & param) {
        DECIMAL dec;
        auto type = param.type();
        switch (type) {
            case Php::Type::Numeric:
                SPA::ToDecimal(param.numericValue(), dec);
                break;
            case Php::Type::Float:
                SPA::ToDecimal(param.floatValue(), dec);
                break;
            case Php::Type::String:
                if (!SPA::ParseDec_long(param.rawValue(), dec)) {
                    throw Php::Exception("Invalid decimal value");
                }
                break;
            default:
                throw Php::Exception("Invalid decimal value");
        }
        *m_pBuffer << dec;
    }

    Php::Value CPhpBuffer::LoadDecimal() {
        EnsureBuffer();
        try{
            DECIMAL dec;
            *m_pBuffer >> dec;
            if (dec.Hi32 || dec.Lo64 > SPA::SAFE_DOUBLE) {
                return SPA::ToString_long(dec);
            }
            return SPA::ToDouble(dec);
        }
        BufferLoadCatch
    }

    void CPhpBuffer::SaveAString(const Php::Value & data) {
        const char *str = data.rawValue();
        std::string s;
        auto type = data.type();
        switch (type) {
            case Php::Type::Undefined:
            case Php::Type::Null:
                *m_pBuffer << str;
                break;
            case Php::Type::False:
            case Php::Type::True:
            case Php::Type::Numeric:
            case Php::Type::Float:
                s = data.stringValue();
                str = s.c_str();
            case Php::Type::String:
                *m_pBuffer << str;
                break;
            default:
                throw Php::Exception("Invalid string value");
        }
    }

    void CPhpBuffer::SaveString(const Php::Value & data) {
        const char *str = data.rawValue();
        std::string s;
        auto type = data.type();
        switch (type) {
            case Php::Type::Undefined:
            case Php::Type::Null:
                str = nullptr;
                *m_pBuffer << (const wchar_t *) str;
                break;
            case Php::Type::False:
            case Php::Type::True:
            case Php::Type::Numeric:
            case Php::Type::Float:
                s = data.stringValue();
                str = s.c_str();
            case Php::Type::String:
                if (str) {
                    SPA::CScopeUQueue sp;
                    SPA::Utilities::ToWide(str, ::strlen(str), *sp);
                    *m_pBuffer << (const wchar_t *) sp->GetBuffer();
                } else {
                    //nullptr string
                    *m_pBuffer << (const wchar_t *) str;
                }
                break;
            default:
                throw Php::Exception("Invalid string value");
        }
    }

    Php::Value CPhpBuffer::SaveString(Php::Parameters & params) {
        EnsureBuffer();
        SaveString(params[0]);
        return this;
    }

    Php::Value CPhpBuffer::LoadString() {
        EnsureBuffer();
        try{
            unsigned int *len = (unsigned int*) m_pBuffer->GetBuffer();
            if (*len == SPA::UQUEUE_NULL_LENGTH) {
                //deal with nullptr string
                unsigned int n;
                *m_pBuffer >> n;
                return nullptr;
            }
            std::wstring s;
            *m_pBuffer >> s;
            SPA::CScopeUQueue sp;
            SPA::Utilities::ToUTF8(s.c_str(), s.size(), *sp);
            return (const char*) sp->GetBuffer();
        }
        BufferLoadCatch
    }

    Php::Value CPhpBuffer::PushBytes(Php::Parameters & params) {
        auto data = params[0].rawValue();
        EnsureBuffer();
        if (data) {
            unsigned int len = (unsigned int) params[0].length();
            unsigned int offset = 0;
            if (params.size() > 2) {
                auto os = params[2].numericValue();
                if (os > 0) {
                    if (os > len) {
                        throw Php::Exception("Bad offset value");
                    } else {
                        offset = (unsigned int) os;
                    }
                }
            }
            len -= offset;
            if (params.size() > 1) {
                auto d = params[1].numericValue();
                if (d > 0 && d < len) {
                    len -= (unsigned int) d;
                }
            }
            m_pBuffer->Push((const unsigned char*) data + offset, len);
        }
        return this;
    }

    Php::Value CPhpBuffer::SaveUUID(Php::Parameters & params) {
        auto data = params[0].rawValue();
        EnsureBuffer();
        if (data) {
            unsigned int len = (unsigned int) params[0].length();
            if (len != sizeof (UUID)) {
                m_pBuffer->SetSize(0);
                throw Php::Exception("Invalid UUID value");
            }
            m_pBuffer->Push((const unsigned char*) data, len);
        } else {
            throw Php::Exception("Invalid UUID value");
        }
        return this;
    }

    Php::Value CPhpBuffer::PopBytes(Php::Parameters & params) {
        EnsureBuffer();
        unsigned int len = m_pBuffer->GetSize();
        if (!len) {
            return "";
        }
        if (params.size()) {
            auto d = params[0].numericValue();
            if (d >= 0 && d < len) {
                len = (unsigned int) d;
            }
        }
        const char *s = (const char*) m_pBuffer->GetBuffer();
        std::string str(s, s + len);
        m_pBuffer->Pop(len);
        return str;
    }

    Php::Value CPhpBuffer::LoadUUID() {
        EnsureBuffer();
        unsigned int len = m_pBuffer->GetSize();
        if (len < sizeof (GUID)) {
            m_pBuffer->SetSize(0);
            throw Php::Exception("Invalid UUID value");
        }
        const char *s = (const char*) m_pBuffer->GetBuffer();
        std::string str(s, s + sizeof (GUID));
        m_pBuffer->Pop((unsigned int) sizeof (GUID));
        return str;
    }

    void CPhpBuffer::SaveObject(const Php::Value &data, const std::string & id) {
        VARTYPE vt;
        auto type = data.type();
        switch (type) {
            case Php::Type::Undefined:
            case Php::Type::Null:
                vt = VT_NULL;
                *m_pBuffer << vt;
                break;
            case Php::Type::Bool:
            case Php::Type::False:
            case Php::Type::True:
            {
                vt = VT_BOOL;
                VARIANT_BOOL b = data.boolValue() ? VARIANT_TRUE : VARIANT_FALSE;
                *m_pBuffer << vt << b;
            }
                break;
            case Php::Type::Numeric:
                if (id == "i" || id == "int") {
                    vt = VT_I4;
                    int n = (int) data.numericValue();
                    *m_pBuffer << vt << n;
                } else if (id == "ui" || id == "uint" || id == "unsigned" || id == "unsigned int") {
                    vt = VT_UI4;
                    unsigned int n = (unsigned int) data.numericValue();
                    *m_pBuffer << vt << n;
                } else if (id == "ul" || id == "ulong" || id == "unsigned long long" || id == "unsigned long int") {
                    vt = VT_UI8;
                    SPA::UINT64 n = (SPA::UINT64)data.numericValue();
                    *m_pBuffer << vt << n;
                } else if (id == "s" || id == "short" || id == "w") {
                    vt = VT_I2;
                    short n = (short) data.numericValue();
                    *m_pBuffer << vt << n;
                } else if (id == "c" || id == "char" || id == "a") {
                    vt = VT_I1;
                    char n = (char) data.numericValue();
                    *m_pBuffer << vt << n;
                } else if (id == "b" || id == "byte" || id == "unsigned char") {
                    vt = VT_UI1;
                    unsigned char n = (unsigned char) data.numericValue();
                    *m_pBuffer << vt << n;
                } else if (id == "us" || id == "ushort" || id == "unsigned short") {
                    vt = VT_UI2;
                    unsigned short n = (unsigned short) data.numericValue();
                    *m_pBuffer << vt << n;
                } else if (id == "dec" || id == "decimal") {
                    vt = VT_DECIMAL;
                    *m_pBuffer << vt;
                    SaveDecimal(data);
                } else {
                    vt = VT_I8;
                    *m_pBuffer << vt << data.numericValue();
                }
                break;
            case Php::Type::Float:
                if (id == "f" || id == "float") {
                    vt = VT_R4;
                    *m_pBuffer << vt << (float) data.floatValue();
                } else if (id == "dec" || id == "decimal") {
                    vt = VT_DECIMAL;
                    *m_pBuffer << vt;
                    SaveDecimal(data);
                } else {
                    vt = VT_R8;
                    *m_pBuffer << vt << data.floatValue();
                }
                break;
            case Php::Type::String:
                if (id == "a" || id == "ascii") {
                    vt = (VT_ARRAY | VT_I1);
                    *m_pBuffer << vt;
                    SaveAString(data);
                } else if ((id == "uuid" || id == "clsid" || id == "guid") && data.length() == sizeof (GUID)) {
                    vt = VT_CLSID;
                    *m_pBuffer << vt;
                    m_pBuffer->Push(data.rawValue(), sizeof (GUID));
                } else if (id == "bytes" || id == "binary") {
                    vt = (VT_ARRAY | VT_UI1);
                    *m_pBuffer << vt;
                    SaveAString(data);
                } else if (id == "dec" || id == "decimal") {
                    vt = VT_DECIMAL;
                    *m_pBuffer << vt;
                    SaveDecimal(data);
                } else {
                    vt = VT_BSTR;
                    *m_pBuffer << vt;
                    SaveString(data);
                }
                break;
            case Php::Type::Array:
                vt = (VT_ARRAY | VT_VARIANT);
            {
                auto d = data.vectorValue<Php::Value>();
                *m_pBuffer << vt << (unsigned int) d.size();
                for (auto it = d.cbegin(), end = d.cend(); it != end; ++it) {
                    SaveObject(*it, id);
                }
            }
                break;
            case Php::Type::Object:
                if (Php::is_a(data, "DateTime")) {
                    vt = VT_DATE;
                    *m_pBuffer << vt;
                    SaveDate(data);
                    break;
                }
            default:
                throw Php::Exception("Unsupported data type");
        }
    }

    Php::Value CPhpBuffer::SaveObject(Php::Parameters & params) {
        std::string id;
        const auto &data = params[0];
        if (params.size() > 1) {
            auto id = params[1].stringValue();
            Trim(id);
            std::transform(id.begin(), id.end(), id.begin(), ::tolower);
        }
        EnsureBuffer();
        SaveObject(data, id);
        return this;
    }

    Php::Value CPhpBuffer::Save(Php::Parameters & params) {
        if (!(params[0].isArray() || params[0].isObject())) {
            throw Php::Exception("An array of data or an object for a complex structure required");
        }
        const Php::Value& callback = params[1];
        callback(params[0], this);
        return this;
    }

    Php::Value CPhpBuffer::Load(Php::Parameters & params) {
        const Php::Value& callback = params[0];
        return callback(this);
    }

    Php::Value CPhpBuffer::LoadObject() {
        EnsureBuffer();
        try{
            VARTYPE vt;
            *m_pBuffer >> vt;
            switch (vt) {
                case VT_NULL:
                case VT_EMPTY:
                    return nullptr;
                case VT_I1:
                {
                    char c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    return c;
                }
                case VT_UI1:
                {
                    unsigned char c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    return c;
                }
                case VT_I2:
                {
                    short c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    return c;
                }
                case VT_BOOL:
                {
                    VARIANT_BOOL c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    return (c != 0);
                }
                case VT_UI2:
                {
                    unsigned short c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    return c;
                }
                case VT_INT:
                case VT_I4:
                {
                    int c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    return c;
                }
                case VT_UINT:
                case VT_UI4:
                {
                    unsigned int c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    return (int64_t) c;
                }
                case VT_UI8:
                case VT_I8:
                {
                    int64_t c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    return c;
                }
                case VT_R4:
                {
                    float c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    return (double) c;
                }
                case VT_R8:
                {
                    double c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    return c;
                }
                case VT_DATE:
                    return LoadDate();
                case VT_CY:
                {
                    int64_t c;
                    m_pBuffer->Pop((unsigned char*) &c, sizeof (c));
                    double cy = (double) c;
                    cy /= 10000;
                    return cy;
                }
                case (VT_ARRAY | VT_UI1):
                case (VT_ARRAY | VT_I1):
                    return LoadAString();
                case VT_BSTR:
                    return LoadString();
                case VT_DECIMAL:
                    return LoadDecimal();
                case VT_CLSID:
                    return LoadUUID();
                default:
                    if ((vt & VT_ARRAY) == VT_ARRAY) {
                        vt = (vt & (~VT_ARRAY));
                        unsigned int count;
                        *m_pBuffer >> count;
                        switch (vt) {
                            case VT_VARIANT:
                            {
                                Php::Array arr;
                                for (unsigned int n = 0; n < count; ++n) {
                                    arr.set((int) n, LoadObject());
                                }
                                return arr;
                            }
                            case VT_BSTR:
                            {
                                std::vector<Php::Value> arr;
                                for (unsigned int n = 0; n < count; ++n) {
                                    arr.push_back(LoadString());
                                }
                                return arr;
                            }
                            case VT_DATE:
                            {
                                std::vector<Php::Value> arr;
                                for (unsigned int n = 0; n < count; ++n) {
                                    arr.push_back(LoadDate());
                                }
                                return arr;
                            }
                            case VT_I2:
                                if (m_pBuffer->GetSize() >= count * sizeof (short)) {
                                    short *p = (short *) m_pBuffer->GetBuffer();
                                    auto arr = std::vector<Php::Value>(p, p + count);
                                    m_pBuffer->Pop(count * sizeof (short));
                                    return arr;
                                } else {
                                    break;
                                }
                            case VT_UI2:
                                if (m_pBuffer->GetSize() >= count * sizeof (unsigned short)) {
                                    unsigned short *p = (unsigned short *) m_pBuffer->GetBuffer();
                                    auto arr = std::vector<Php::Value>(p, p + count);
                                    m_pBuffer->Pop(count * sizeof (unsigned short));
                                    return arr;
                                } else {
                                    break;
                                }
                            case VT_R4:
                                if (m_pBuffer->GetSize() >= count * sizeof (float)) {
                                    float *p = (float *) m_pBuffer->GetBuffer();
                                    std::vector<Php::Value> arr;
                                    for (unsigned int n = 0; n < count; ++n) {
                                        arr.push_back((double) p[n]);
                                    }
                                    m_pBuffer->Pop(count * sizeof (float));
                                    return arr;
                                } else {
                                    break;
                                }
                            case VT_R8:
                                if (m_pBuffer->GetSize() >= count * sizeof (double)) {
                                    double *p = (double *) m_pBuffer->GetBuffer();
                                    std::vector<Php::Value> arr;
                                    for (unsigned int n = 0; n < count; ++n) {
                                        arr.push_back(p[n]);
                                    }
                                    m_pBuffer->Pop(count * sizeof (double));
                                    return arr;
                                } else {
                                    break;
                                }
                            case VT_INT:
                            case VT_I4:
                                if (m_pBuffer->GetSize() >= count * sizeof (int)) {
                                    int *p = (int *) m_pBuffer->GetBuffer();
                                    auto arr = std::vector<Php::Value>(p, p + count);
                                    m_pBuffer->Pop(count * sizeof (int));
                                    return arr;
                                } else {
                                    break;
                                }
                            case VT_UI4:
                            case VT_UINT:
                                if (m_pBuffer->GetSize() >= count * sizeof (unsigned int)) {
                                    unsigned int *p = (unsigned int *) m_pBuffer->GetBuffer();
                                    std::vector<Php::Value> arr;
                                    for (unsigned int n = 0; n < count; ++n) {
                                        arr.push_back((int64_t) p[n]);
                                    }
                                    m_pBuffer->Pop(count * sizeof (unsigned int));
                                    return arr;
                                } else {
                                    break;
                                }
                            case VT_I8:
                            case VT_UI8:
                                if (m_pBuffer->GetSize() >= count * sizeof (int64_t)) {
                                    int64_t *p = (int64_t *) m_pBuffer->GetBuffer();
                                    auto arr = std::vector<Php::Value>(p, p + count);
                                    m_pBuffer->Pop(count * sizeof (int64_t));
                                    return arr;
                                } else {
                                    break;
                                }
                            case VT_BOOL:
                                if (m_pBuffer->GetSize() >= count * sizeof (VARIANT_BOOL)) {
                                    std::vector<Php::Value> arr;
                                    VARIANT_BOOL *p = (VARIANT_BOOL *) m_pBuffer->GetBuffer();
                                    for (unsigned int n = 0; n < count; ++n) {
                                        arr.push_back(p[n] ? true : false);
                                    }
                                    m_pBuffer->Pop(count * sizeof (VARIANT_BOOL));
                                    return arr;
                                } else {
                                    break;
                                }
                            case VT_DECIMAL:
                                if (m_pBuffer->GetSize() >= count * sizeof (DECIMAL)) {
                                    std::vector<Php::Value> arr;
                                    DECIMAL *p = (DECIMAL *) m_pBuffer->GetBuffer();
                                    for (unsigned int n = 0; n < count; ++n) {
                                        DECIMAL &dec = p[n];
                                        if (dec.Hi32 || dec.Lo64 > SPA::SAFE_DOUBLE) {
                                            arr.push_back(SPA::ToString_long(dec));
                                        } else {
                                            arr.push_back(SPA::ToDouble(dec));
                                        }
                                    }
                                    m_pBuffer->Pop(count * sizeof (DECIMAL));
                                    return arr;
                                } else {
                                    break;
                                }
                            case VT_CY:
                                if (m_pBuffer->GetSize() >= count * sizeof (CY)) {
                                    std::vector<Php::Value> arr;
                                    CY *p = (CY *) m_pBuffer->GetBuffer();
                                    for (unsigned int n = 0; n < count; ++n) {
                                        double d = (double) (p[n].int64);
                                        d /= 10000;
                                        arr.push_back(d);
                                    }
                                    m_pBuffer->Pop(count * sizeof (CY));
                                    return arr;
                                } else {
                                    break;
                                }
                            default:
                                break;
                        }
                    }
                    break;
            }
            m_pBuffer->SetSize(0);
            throw Php::Exception("Invalid data type for deserialization");
        }
        BufferLoadCatch
    }

    void CPhpBuffer::RegisterInto(Php::Namespace & spa) {
        Php::Class<CPhpBuffer> buffer(PHP_BUFFER);
        buffer.property("DEFAULT_BUFFER_SIZE", (int64_t) SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE, Php::Const);
        buffer.property("DEFAULT_BLOCK_SIZE", (int64_t) SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE, Php::Const);
        buffer.method<&CPhpBuffer::__construct>(PHP_CONSTRUCT);
        buffer.method<&CPhpBuffer::Empty>(PHP_EMPTY);
        buffer.method<&CPhpBuffer::CleanTrack>("CleanTrack");
        buffer.method<&CPhpBuffer::Discard>("Discard",{
            Php::ByVal(PHP_LEN, Php::Type::Numeric)
        });
        buffer.method<&CPhpBuffer::SaveDate>("SaveDate",{
            Php::ByVal("dt", "DateTime", false, true)
        });
        buffer.method<&CPhpBuffer::LoadDate>("LoadDate");
        buffer.method<&CPhpBuffer::SaveByte>("SaveByte",{
            Php::ByVal("b", Php::Type::Numeric)
        });
        buffer.method<&CPhpBuffer::LoadByte>("LoadByte");
        buffer.method<&CPhpBuffer::SaveAChar>("SaveAChar",{
            Php::ByVal("c", Php::Type::Numeric)
        });
        buffer.method<&CPhpBuffer::LoadAChar>("LoadAChar");
        buffer.method<&CPhpBuffer::SaveBool>("SaveBool",{
            Php::ByVal("b")
        });
        buffer.method<&CPhpBuffer::LoadBool>("LoadBool");
        buffer.method<&CPhpBuffer::SaveShort>("SaveShort",{
            Php::ByVal("s", Php::Type::Numeric)
        });
        buffer.method<&CPhpBuffer::LoadShort>("LoadShort");
        buffer.method<&CPhpBuffer::SaveInt>("SaveInt",{
            Php::ByVal("i", Php::Type::Numeric)
        });
        buffer.method<&CPhpBuffer::LoadInt>("LoadInt");
        buffer.method<&CPhpBuffer::SaveUInt>("SaveUInt",{
            Php::ByVal("ui", Php::Type::Numeric)
        });
        buffer.method<&CPhpBuffer::LoadUInt>("LoadUInt");
        buffer.method<&CPhpBuffer::SaveUShort>("SaveUShort",{
            Php::ByVal("us", Php::Type::Numeric)
        });
        buffer.method<&CPhpBuffer::LoadUShort>("LoadUShort");
        buffer.method<&CPhpBuffer::SaveLong>("SaveLong",{
            Php::ByVal("l", Php::Type::Numeric)
        });
        buffer.method<&CPhpBuffer::LoadLong>("LoadLong");
        buffer.method<&CPhpBuffer::SaveLong>("SaveULong",{
            Php::ByVal("ul", Php::Type::Numeric)
        });
        buffer.method<&CPhpBuffer::LoadLong>("LoadULong");
        buffer.method<&CPhpBuffer::SaveUShort>("SaveChar",{
            Php::ByVal("wc", Php::Type::Numeric)
        });
        buffer.method<&CPhpBuffer::LoadUShort>("LoadChar");
        buffer.method<&CPhpBuffer::SaveDouble>("SaveDouble",{
            Php::ByVal("d", Php::Type::Float)
        });
        buffer.method<&CPhpBuffer::LoadDouble>("LoadDouble");
        buffer.method<&CPhpBuffer::SaveFloat>("SaveFloat",{
            Php::ByVal("f", Php::Type::Float)
        });
        buffer.method<&CPhpBuffer::LoadFloat>("LoadFloat");
        buffer.method<&CPhpBuffer::SaveAString>("SaveAString",{
            Php::ByVal("a", Php::Type::Null) //ASCII string
        });
        buffer.method<&CPhpBuffer::LoadAString>("LoadAString");
        buffer.method<&CPhpBuffer::SaveString>("SaveString",{
            Php::ByVal("w", Php::Type::Null) //UNICODE string
        });
        buffer.method<&CPhpBuffer::LoadString>("LoadString");
        buffer.method<&CPhpBuffer::SaveDecimal>("SaveDecimal",{
            Php::ByVal("dec", Php::Type::Null)
        });
        buffer.method<&CPhpBuffer::LoadDecimal>("LoadDecimal");
        buffer.method<&CPhpBuffer::PushBytes>("PushBytes",{
            Php::ByVal("bytes", Php::Type::String), //ASCII string
            Php::ByVal(PHP_LEN, Php::Type::Numeric, false),
            Php::ByVal("offset", Php::Type::Numeric, false)
        });
        buffer.method<&CPhpBuffer::PopBytes>(PHP_POPBYTES,{
            Php::ByVal(PHP_LEN, Php::Type::Numeric, false)
        });
        buffer.method<&CPhpBuffer::SaveUUID>("SaveUUID",{
            Php::ByVal("uuid", Php::Type::String) //ASCII string
        });
        buffer.method<&CPhpBuffer::LoadUUID>("LoadUUID");
        buffer.method<&CPhpBuffer::SaveObject>("SaveObject",{
            Php::ByVal(PHP_OBJ, Php::Type::Null, true),
            Php::ByVal(PHP_OBJ, Php::Type::String, false)
        });
        buffer.method<&CPhpBuffer::LoadObject>("LoadObject");
        buffer.method<&CPhpBuffer::Save>("Save",{
            Php::ByVal(PHP_OBJ, Php::Type::Null),
            Php::ByVal("func", Php::Type::Callable)
        });
        buffer.method<&CPhpBuffer::Load>("Load",{
            Php::ByVal("func", Php::Type::Callable)
        });
        spa.add(buffer);
    }

    Php::Value CPhpBuffer::__get(const Php::Value & name) {
        EnsureBuffer();
        if (name == PHP_SIZE) {
            return (int64_t) (m_pBuffer->GetSize());
        } else if (name == "Head" || name == "HeadPosition") {
            return (int64_t) m_pBuffer->GetHeadPosition();
        } else if (name == "BufferSize" || name == "MaxSize" || name == "MaxBufferSize") {
            return (int64_t) m_pBuffer->GetMaxSize();
        } else if (name == "Tail" || name == "TailSize") {
            return (int64_t) m_pBuffer->GetTailSize();
        } else if (name == "OS") {
            return (int64_t) m_pBuffer->GetOS();
        } else if (name == "Endian") {
            return m_pBuffer->GetEndian();
        } else if (name == PHP_POINTER_ADDRESS) {
            return (int64_t) m_pBuffer;
        }
        return Php::Base::__get(name);
    }

    void CPhpBuffer::__set(const Php::Value &name, const Php::Value & value) {
        if (!m_pBuffer) {
            auto size = value.numericValue();
            if (name == PHP_SIZE && size > 0) {
                m_pBuffer = SPA::CScopeUQueue::Lock(SPA::GetOS(), SPA::IsBigEndian(), (unsigned int) size);
            } else {
                m_pBuffer = SPA::CScopeUQueue::Lock();
            }
        }
        if (name == PHP_SIZE) {
            auto size = value.numericValue();
            if (size < 0) {
                size = 0;
            }
            if ((unsigned int) size > m_pBuffer->GetSize() + m_pBuffer->GetHeadPosition()) {
                throw Php::Exception("Invalid size value");
            }
            m_pBuffer->SetSize((unsigned int) size);
        } else if (name == "OS") {
            auto os = value.numericValue();
            if (os < SPA::osWin || os > SPA::osWinPhone) {
                throw Php::Exception("Bad operation system value");
            }
            m_pBuffer->SetOS((SPA::tagOperationSystem)os);
        } else if (name == "Endian") {
            auto endian = value.boolValue();
            m_pBuffer->SetEndian(endian);
        } else {
            Php::Base::__set(name, value);
        }
    }

} //namespace PA