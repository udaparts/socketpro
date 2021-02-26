#ifndef _SOCKETPRO_JSON_VALUE_H_
#define _SOCKETPRO_JSON_VALUE_H_

#include "commutil.h"
#ifndef WIN32_64
#include <math.h>
#endif
#include <map>
#include <vector>

namespace SPA {
    namespace JSON {
        namespace Internal {

            static bool SkipWhitespace(const char*& data) {
                while (*data != 0 && (*data == ' ' || *data == '\t' || *data == '\r' || *data == '\n')) {
                    ++data;
                }
                return *data != 0;
            }

            static bool AtLeastChars(const char* s, size_t n) {
                if (!s) return false;
                const char* str = s;
                for (; *str && n; ++str, --n) {
                }
                return (n == 0 || *str > 0);
            }

            static double ParseDecimal(const char*& data) {
                double decimal = 0, factor = 0.1;
                while (*data != 0 && *data >= '0' && *data <= '9') {
                    int digit = (*data++ -'0');
                    decimal = decimal + digit * factor;
                    factor *= 0.1;
                }
                return decimal;
            }

            static bool ExtractString(const char*& data, std::string& str) {
                const char* start = data;
                while (*data != 0) {
                    char next = *data;
                    if (next == '\\') {
                        ++data;
                        switch (*data) {
                            case '"': case '\\': case 'b': case 'f':
                            case 'n': case 'r': case 't':
                                next = *data;
                                break;
                            case 'u':
                                if (!AtLeastChars(data, 5)) return false;
                                next = 0;
                                for (int i = 0; i < 4; ++i) {
                                    ++data;
                                    next <<= 4;
                                    if (*data >= '0' && *data <= '9')
                                        next |= (*data - '0');
                                    else if (*data >= 'A' && *data <= 'F')
                                        next |= (10 + (*data - 'A'));
                                    else if (*data >= 'a' && *data <= 'f')
                                        next |= (10 + (*data - 'a'));
                                    else {
                                        return false;
                                    }
                                }
                                break;
                            default:
                                return false;
                        }
                    } else if (next == '"') {
                        str.append(start, data);
                        ++data;
                        return true;
                    } else if (next < ' ' && next != '\t') {
                        return false;
                    }
                    ++data;
                }
                return false;
            }
        }

        enum class enumType {
            Null, String, Bool, Number, Array, Object, Int64
        };

        class JValue;

        typedef std::map<std::string, JValue> JObject;
        typedef std::vector<JValue> JArray;

        static JValue* Parse(const char* data);

        static std::string&& Escape(std::string& str) {
            for (size_t pos = 0, len = str.size(); pos < len; ++pos) {
                switch (str[pos]) {
                    case '\\': case '\b': case '\f': case '"':
                    case '\t': case '\r': case '\n':
                        str.insert(pos, 1, '\\');
                        ++pos;
                        ++len;
                        break;
                    default:
                        break;
                }
            }
            return std::move(str);
        }

        static std::string Escape(const char *str) {
            std::string s(str ? str : "");
            return Escape(s);
        }

        static std::string&& Unescape(std::string& str) {
            char prev = 0;
            for (size_t pos = 0, len = str.size(); pos < len; ++pos) {
                if (prev == '\\') {
                    switch (str[pos]) {
                    case '\\': case '\b': case '\f': case '"':
                    case '\t': case '\r': case '\n':
                        str.erase(--pos, 1);
                        --len;
                        break;
                    default:
                        break;
                    }
                }
                prev = str[pos];
            }
            return std::move(str);
        }

        static std::string Unescape(const char* str) {
            std::string s(str ? str : "");
            return Unescape(s);
        }

        class JValue {
        public:

            JValue() noexcept : type(enumType::Null) {
            }

            JValue(const char* str, bool escape = true) {
                if (str) {
                    type = enumType::String;
                    strValue = new std::string(str);
                    if (escape) Escape(*strValue);
                } else {
                    type = enumType::Null;
                }
            }

            JValue(const std::string& str, bool escape = true) : type(enumType::String), strValue(new std::string(str)) {
                if (escape) Escape(*strValue);
            }

            JValue(bool value) noexcept : type(enumType::Bool), bValue(value) {
            }

            JValue(double d) noexcept : type(enumType::Number), dValue(d) {
            }

            JValue(unsigned int n) : type(enumType::Int64), int64Value(n) {
            }

            JValue(int n) noexcept : type(enumType::Int64), int64Value(n) {
            }

            JValue(INT64 n) noexcept : type(enumType::Int64), int64Value(n) {
            }

            JValue(const JArray& arr) : type(enumType::Array), arrValue(new JArray(arr)) {
            }

            JValue(const JObject& obj) : type(enumType::Object), objValue(new JObject(obj)) {
            }

            JValue(const JValue& src) : type(src.type) {
                switch (type) {
                    case enumType::String:
                        strValue = new std::string(*src.strValue);
                        break;
                    case enumType::Bool:
                        bValue = src.bValue;
                        break;
                    case enumType::Int64:
                        int64Value = src.int64Value;
                        break;
                    case enumType::Number:
                        dValue = src.dValue;
                        break;
                    case enumType::Array:
                        arrValue = new JArray(*src.arrValue);
                        break;
                    case enumType::Object:
                        objValue = new JObject(*src.objValue);
                        break;
                    default: //null
                        break;
                }
            }

            JValue(JArray&& arr) : type(enumType::Array), arrValue(new JArray) {
                arrValue->swap(arr);
            }

            JValue(JObject&& obj) : type(enumType::Object), objValue(new JObject) {
                objValue->swap(obj);
            }

            JValue(std::string&& src, bool escape = true) : type(enumType::String), strValue(new std::string) {
                strValue->swap(src);
                if (escape) Escape(*strValue);
            }

            JValue(JValue&& src) noexcept : type(src.type), int64Value(src.int64Value) {
                src.type = enumType::Null;
            }

            ~JValue() {
                Clean();
            }

        public:
            static const unsigned int DEFAULT_INDENT_CHARS = 2;

            inline enumType GetType() const noexcept {
                return type;
            }

            inline std::string& AsString() const noexcept {
                assert(type == enumType::String);
                return (*strValue);
            }

            inline bool AsBool() const noexcept {
                switch (type) {
                    case enumType::Bool:
                        return bValue;
                    case enumType::Number:
                        return dValue;
                    case enumType::Int64:
                        return int64Value;
                    case enumType::Array:
                        return arrValue->size();
                    case enumType::Object:
                        return objValue->size();
                    case enumType::String:
                        return strValue->size();
                    default: //null
                        break;
                }
                return false;
            }

            inline double AsNumber() const noexcept {
                switch (type) {
                    case enumType::Bool:
                        return bValue ? 1.0 : 0.0;
                    case enumType::Number:
                        return dValue;
                    case enumType::Int64:
                        return (double) int64Value;
                    default:
                        assert(false); //unexpected error
                        break;
                }
                return 0.0;
            }

            inline INT64 AsInt64() const noexcept {
                switch (type) {
                    case enumType::Bool:
                        return bValue ? 1 : 0;
                    case enumType::Number:
                        return (INT64) dValue;
                    case enumType::Int64:
                        return int64Value;
                    default:
                        assert(false); //unexpected error
                        break;
                }
                return 0;
            }

            inline JArray& AsArray() const noexcept {
                assert(type == enumType::Array);
                return (*arrValue);
            }

            inline JObject& AsObject() const noexcept {
                assert(type == enumType::Object);
                return (*objValue);
            }

            inline std::size_t Size() const noexcept {
                switch (type) {
                    case enumType::Array:
                        return arrValue->size();
                    case enumType::Object:
                        return objValue->size();
                    default:
                        break;
                }
                return 0;
            }

            JValue* Child(std::size_t index) const noexcept {
                if (type != enumType::Array) return nullptr;
                if (index < arrValue->size()) return &arrValue->at(index);
                return nullptr;
            }

            JValue* Child(const char* name) const noexcept {
                if (type != enumType::Object) return nullptr;
                JObject::iterator it = objValue->find(name);
                if (it != objValue->end()) return &(it->second);
                return nullptr;
            }

            std::vector<std::string> ObjKeys() const {
                std::vector<std::string> keys;
                if (type == enumType::Object) {
                    for (auto it = objValue->cbegin(), end = objValue->cend(); it != end; ++it) {
                        keys.push_back(it->first);
                    }
                }
                return std::move(keys);
            }

            std::string Stringify(bool pretty = true, const char* df = "%lf", unsigned int indent_chars = DEFAULT_INDENT_CHARS) const {
                std::string ret_str;
                unsigned int indent = 0;
                Stringify(ret_str, indent, pretty, df, indent_chars);
                return std::move(ret_str);
            }

            JValue& operator=(JArray&& arr) {
                if (type == enumType::Array) {
                    arrValue->swap(arr);
                } else {
                    Clean();
                    type = enumType::Array;
                    arrValue = new JArray;
                    arrValue->swap(arr);
                }
                return *this;
            }

            JValue& operator=(JObject&& obj) {
                if (type == enumType::Object) {
                    objValue->swap(obj);
                } else {
                    Clean();
                    type = enumType::Object;
                    objValue = new JObject;
                    objValue->swap(obj);
                }
                return *this;
            }

            JValue& operator=(std::string&& src) {
                if (type == enumType::String) {
                    strValue->swap(src);
                } else {
                    Clean();
                    type = enumType::String;
                    strValue = new std::string;
                    strValue->swap(src);
                }
                Escape(*strValue);
                return *this;
            }

            JValue& operator=(JValue&& src) noexcept {
                if (this == &src) return *this;
                enumType t = type;
                INT64 int64 = int64Value;
                type = src.type;
                int64Value = src.int64Value;
                src.type = t;
                src.int64Value = int64;
                return *this;
            }

            JValue& operator=(bool b) {
                Clean();
                type = enumType::Bool;
                bValue = b;
                return *this;
            }

            JValue& operator=(double d) {
                Clean();
                type = enumType::Number;
                dValue = d;
                return *this;
            }

            JValue& operator=(int n) {
                Clean();
                type = enumType::Int64;
                int64Value = n;
                return *this;
            }

            JValue& operator=(unsigned int n) {
                Clean();
                type = enumType::Int64;
                int64Value = n;
                return *this;
            }

            JValue& operator=(INT64 n) {
                Clean();
                type = enumType::Int64;
                int64Value = n;
                return *this;
            }

            JValue& operator=(const char* str) {
                Clean();
                if (str) {
                    type = enumType::String;
                    strValue = new std::string(str);
                    Escape(*strValue);
                } else {
                    type = enumType::Null;
                }
                return *this;
            }

            JValue& operator=(const std::string& str) {
                Clean();
                type = enumType::String;
                strValue = new std::string(str);
                Escape(*strValue);
                return *this;
            }

            JValue& operator=(const JArray& arr) {
                Clean();
                type = enumType::Array;
                arrValue = new JArray(arr);
                return *this;
            }

            JValue& operator=(const JObject& obj) {
                Clean();
                type = enumType::Object;
                objValue = new JObject(obj);
                return *this;
            }

            JValue& operator=(const JValue& src) {
                if (this == &src) return *this;
                Clean();
                type = src.type;
                switch (type) {
                    case enumType::String:
                        strValue = new std::string(*src.strValue);
                        break;
                    case enumType::Bool:
                        bValue = src.bValue;
                        break;
                    case enumType::Int64:
                        int64Value = src.int64Value;
                        break;
                    case enumType::Number:
                        dValue = src.dValue;
                        break;
                    case enumType::Array:
                        arrValue = new JArray(*src.arrValue);
                        break;
                    case enumType::Object:
                        objValue = new JObject(*src.objValue);
                        break;
                    default: //null
                        break;
                }
                return *this;
            }

        private:

            void Stringify(std::string& ret_str, unsigned int& indent, bool pretty = true, const char* df = "%lf", unsigned int indent_chars = DEFAULT_INDENT_CHARS) const {
                switch (type) {
                    case enumType::Int64:
                        ret_str += std::to_string(int64Value);
                        break;
                    case enumType::String:
                        ret_str.push_back('"');
                        ret_str.append(*strValue);
                        ret_str.push_back('"');
                        break;
                    case enumType::Bool:
                        ret_str += (bValue ? "true" : "false");
                        break;
                    case enumType::Number:
                        if (isinf(dValue) || isnan(dValue)) {
                            ret_str += "null";
                        } else {
                            char str[64]; //don't give me a too long precision!!!
#ifdef WIN32_64
                            ::sprintf_s(str, df, dValue);
#else
                            sprintf(str, df, dValue);
#endif
                            ret_str += str;
                        }
                        break;
                    case enumType::Array:
                    {
                        ret_str.push_back('[');
                        if (pretty) ret_str.push_back('\n');
                        ++indent;
                        JArray::const_iterator iter = arrValue->begin(), end = arrValue->end();
                        while (iter != end) {
                            if (pretty) ret_str.append((size_t) indent * indent_chars, ' ');
                            iter->Stringify(ret_str, indent, pretty, df, indent_chars);
                            if (++iter != end) {
                                ret_str.push_back(',');
                                if (pretty) ret_str.push_back('\n');
                            }
                        }
                        --indent;
                        if (pretty) {
                            ret_str.push_back('\n');
                            ret_str.append((size_t) indent * indent_chars, ' ');
                        }
                        ret_str.push_back(']');
                        break;
                    }
                    case enumType::Object:
                    {
                        ret_str.push_back('{');
                        if (pretty) ret_str.push_back('\n');
                        ++indent;
                        JObject::const_iterator iter = objValue->begin(), end = objValue->end();
                        while (iter != end) {
                            if (pretty) ret_str.append((size_t) indent * indent_chars, ' ');
                            ret_str.push_back('"');
                            ret_str.append(iter->first);
                            ret_str += "\":";
                            iter->second.Stringify(ret_str, indent, pretty, df, indent_chars);
                            if (++iter != end) {
                                ret_str.push_back(',');
                                if (pretty) ret_str.push_back('\n');
                            }
                        }
                        --indent;
                        if (pretty) {
                            ret_str.push_back('\n');
                            ret_str.append((size_t) indent * indent_chars, ' ');
                        }
                        ret_str.push_back('}');
                        break;
                    }
                    default: //null
                        ret_str += "null";
                        break;
                }
            }

            static JValue* Parse(const char*& data) {
                if (*data == '"') {
                    std::string str;
                    if (!Internal::ExtractString(++data, str)) return nullptr;
                    return new JValue(std::move(str), false);
                } else if ((Internal::AtLeastChars(data, 4) && ::strncmp(data, "true", 4) == 0) || (Internal::AtLeastChars(data, 5) && ::strncmp(data, "false", 5) == 0)) {
                    bool value = (::strncmp(data, "true", 4) == 0);
                    data += value ? 4 : 5;
                    return new JValue(value);
                } else if (Internal::AtLeastChars(data, 4) && ::strncmp(data, "null", 4) == 0) {
                    data += 4;
                    return new JValue();
                } else if (*data == '-' || (*data >= '0' && *data <= '9')) {
                    bool neg = *data == '-';
                    if (neg) ++data;
                    INT64 int64 = 0;
                    if (*data == '0') {
                        ++data;
                    } else if (*data >= '1' && *data <= '9') {
                        int64 = atoll(data, data);
                    } else {
                        return nullptr;
                    }
                    if (*data != '.' && *data != 'e' && *data != 'E') return new JValue(neg ? -int64 : int64);
                    double number = int64;
                    if (*data == '.') {
                        ++data;
                        if (!(*data >= '0' && *data <= '9')) return nullptr;
                        number += Internal::ParseDecimal(data);
                    }
                    if (*data == 'E' || *data == 'e') {
                        ++data;
                        bool neg_expo = false;
                        if (*data == '-' || *data == '+') {
                            neg_expo = (*data == '-');
                            ++data;
                        }
                        if (!(*data >= '0' && *data <= '9')) return nullptr;
                        double expo = atoll(data, data);
                        for (double i = 0.0; i < expo; ++i) {
                            number = neg_expo ? (number / 10) : (number * 10);
                        }
                    }
                    return new JValue(neg ? -number : number);
                } else if (*data == '{') {
                    JObject object;
                    ++data;
                    while (*data) {
                        if (!Internal::SkipWhitespace(data)) return nullptr;
                        if (object.size() == 0 && *data == '}') {
                            ++data;
                            return new JValue(object);
                        }
                        if (*data != '"') return nullptr;
                        std::string name;
                        if (!Internal::ExtractString(++data, name)) return nullptr;
                        if (!Internal::SkipWhitespace(data)) return nullptr;
                        if (*data++ != ':') return nullptr;
                        if (!Internal::SkipWhitespace(data)) return nullptr;
                        JValue* value = Parse(data);
                        if (value == nullptr) return nullptr;
                        object[std::move(name)] = std::move(*value);
                        delete value;
                        if (!Internal::SkipWhitespace(data)) return nullptr;
                        if (*data == '}') {
                            ++data;
                            return new JValue(std::move(object));
                        }
                        if (*data != ',') return nullptr;
                        ++data;
                    }
                    return nullptr;
                } else if (*data == '[') {
                    JArray array;
                    ++data;
                    while (*data) {
                        if (!Internal::SkipWhitespace(data)) return nullptr;
                        if (array.size() == 0 && *data == ']') {
                            ++data;
                            return new JValue(array);
                        }
                        JValue* value = Parse(data);
                        if (value == nullptr) return nullptr;
                        array.push_back(std::move(*value));
                        delete value;
                        if (!Internal::SkipWhitespace(data)) return nullptr;
                        if (*data == ']') {
                            ++data;
                            return new JValue(std::move(array));
                        }
                        if (*data != ',') return nullptr;
                        ++data;
                    }
                    return nullptr;
                }
                return nullptr;
            }

            inline void Clean() {
                switch (type) {
                    case enumType::Array:
                        delete arrValue;
                        break;
                    case enumType::Object:
                        delete objValue;
                        break;
                    case enumType::String:
                        delete strValue;
                        break;
                    default:
                        break;
                }
            }

            enumType type;

            union {
                bool bValue;
                INT64 int64Value;
                double dValue;
                std::string* strValue;
                JArray* arrValue;
                JObject* objValue;
            };
            friend JValue* Parse(const char* data);
        };

        static JValue* Parse(const char* data) {
            if (!data) return nullptr;
            if (!Internal::SkipWhitespace(data)) return nullptr;
            JValue* value = JValue::Parse(data);
            if (value == nullptr) return nullptr;
            if (Internal::SkipWhitespace(data)) {
                delete value;
                return nullptr;
            }
            return value;
        }

        static JValue* ParseFromFile(const char* filePath, int &errCode) {
            errCode = 0;
#ifdef WIN32_64
            FILE* f = nullptr;
            errCode = ::fopen_s(&f, filePath, "r");
            if (!f) return nullptr;
            std::shared_ptr<FILE> fp(f, [](FILE* f) {
                if (f) {
                    ::fclose(f);
                }
            });
#else
            std::shared_ptr<FILE> fp(fopen(filePath, "r"), [](FILE* f) {
                if (f) {
                    ::fclose(f);
                }
            });
            if (!fp) {
                errCode = errno;
                return nullptr;
            }
#endif
            fseek(fp.get(), 0, SEEK_END);
            long size = ::ftell(fp.get());
            fseek(fp.get(), 0, SEEK_SET);
            SPA::CScopeUQueue sb(SPA::GetOS(), SPA::IsBigEndian(), (unsigned int)size + sizeof(wchar_t));
            auto res = ::fread((char*)sb->GetBuffer(), 1, (size_t)size, fp.get());
            fp.reset();
            sb->SetSize((unsigned int)res);
            sb->SetNull();
            unsigned char byte_order_mark[] = { 0xef, 0xbb, 0xbf };
            const char* json = (const char*)sb->GetBuffer();
            if (res >= 3) {
                //UTF8 BOM
                if (::memcmp(byte_order_mark, json, sizeof(byte_order_mark)) == 0) json += 3;
            }
            return Parse(json);
        }
    } //namespace JSON
} //namespace SPA

#endif
