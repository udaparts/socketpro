#ifndef _SOCKETPRO_JSON_VALUE_H_
#define _SOCKETPRO_JSON_VALUE_H_

#include "membuffer.h"
#include <map>
#include <vector>

namespace SPA {
    namespace JSON {

        template<typename TChar>
        std::basic_string<TChar>&& Escape(std::basic_string<TChar>& str, bool escape_fowardslash = false) {
            for (size_t pos = 0, len = str.size(); pos < len; ++pos) {
                switch (str[pos]) {
                    case '\\': case '\b': case '\f': case '"':
                    case '\t': case '\r': case '\n':
                        str.insert(pos, 1, '\\');
                        ++pos;
                        ++len;
                        break;
                    case '/':
                        if (escape_fowardslash) {
                            str.insert(pos, 1, '\\');
                            ++pos;
                            ++len;
                        }
                    default:
                        break;
                }
            }
            return std::move(str);
        }

        template<typename TChar>
        std::basic_string<TChar> Escape(const TChar* str, bool escape_fowardslash = false) {
            std::basic_string<TChar> s;
            if (str) s.assign(str);
            return Escape(s, escape_fowardslash);
        }

        template<typename TChar>
        std::basic_string<TChar>&& Unescape(std::basic_string<TChar>& str) {
            char prev = 0;
            for (size_t pos = 0, len = str.size(); pos < len; ++pos) {
                if (prev == '\\') {
                    switch (str[pos]) {
                        case '\\': case '\b': case '\f': case '"':
                        case '\t': case '\r': case '\n': case '/':
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

        template<typename TChar>
        std::basic_string<TChar> Unescape(const TChar* str) {
            std::basic_string<TChar> s;
            if (str) s.assign(str);
            return Unescape(s);
        }

        enum class enumType {
            Null, String, Bool, Number, Array, Object, Int64, Uint64
        };

        template <typename TChar>
        class JValue;

        template<typename TChar> using JObject = std::map<std::basic_string<TChar>, JValue<TChar>>;
        template<typename TChar> using JArray = std::vector<JValue<TChar>>;

        template <typename TChar>
        class JValue {
        public:
            typedef std::basic_string<TChar> JString;

            JValue() noexcept : type(enumType::Null) {
            }

            JValue(const TChar* str, bool escape = true, bool escape_fowardslash = false) {
                if (str) {
                    type = enumType::String;
                    strValue = new JString(str);
                    if (escape) Escape(*strValue, escape_fowardslash);
                } else {
                    type = enumType::Null;
                }
            }

            JValue(const JString& str, bool escape = true, bool escape_fowardslash = false) : type(enumType::String), strValue(new JString(str)) {
                if (escape) Escape(*strValue, escape_fowardslash);
            }

            JValue(bool value) noexcept : type(enumType::Bool), bValue(value) {
            }

            JValue(double d) noexcept : type(enumType::Number), dValue(d) {
            }

            JValue(unsigned int n) noexcept : type(enumType::Uint64), uint64Value(n) {
            }

            JValue(UINT64 n) noexcept : type(enumType::Uint64), uint64Value(n) {
            }

            JValue(int n) noexcept : type(enumType::Uint64), int64Value(n) {
                if (n < 0) type = enumType::Int64;
            }

            JValue(INT64 n) noexcept : type(enumType::Uint64), int64Value(n) {
                if (n < 0) type = enumType::Int64;
            }

            JValue(const JArray<TChar>& arr) : type(enumType::Array), arrValue(new JArray<TChar>(arr)) {
            }

            JValue(const JObject<TChar>& obj) : type(enumType::Object), objValue(new JObject<TChar>(obj)) {
            }

            JValue(const JValue& src) : type(src.type) {
                switch (type) {
                    case enumType::String:
                        strValue = new JString(*src.strValue);
                        break;
                    case enumType::Bool:
                        bValue = src.bValue;
                        break;
                    case enumType::Int64:
                        int64Value = src.int64Value;
                        break;
                    case enumType::Uint64:
                        uint64Value = src.uint64Value;
                        break;
                    case enumType::Number:
                        dValue = src.dValue;
                        break;
                    case enumType::Array:
                        arrValue = new JArray<TChar>(*src.arrValue);
                        break;
                    case enumType::Object:
                        objValue = new JObject<TChar>(*src.objValue);
                        break;
                    default: //null
                        break;
                }
            }

            JValue(JArray<TChar>&& arr) : type(enumType::Array), arrValue(new JArray<TChar>) {
                arrValue->swap(arr);
            }

            JValue(JObject<TChar>&& obj) : type(enumType::Object), objValue(new JObject<TChar>) {
                objValue->swap(obj);
            }

            JValue(JString&& src, bool escape = true, bool escape_fowardslash = false) : type(enumType::String), strValue(new JString) {
                strValue->swap(src);
                if (escape) Escape(*strValue, escape_fowardslash);
            }

            JValue(JValue&& src) noexcept : type(src.type), int64Value(src.int64Value) {
                src.type = enumType::Null;
            }

            ~JValue() {
                Clean();
            }

        public:
            static const unsigned int DEFAULT_INDENT_CHARS = 2;
            static const unsigned int DEFAULT_DOUBLE_PRECISION = 16;

            inline enumType GetType() const noexcept {
                return type;
            }

            inline JString& AsString() const {
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
                    case enumType::Uint64:
                        return uint64Value;
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
                    case enumType::Uint64:
                        return (double) uint64Value;
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
                    case enumType::Uint64:
                        return int64Value;
                    default:
                        assert(false); //unexpected error
                        break;
                }
                return 0;
            }

            inline UINT64 AsUint64() const noexcept {
                switch (type) {
                    case enumType::Bool:
                        return bValue ? 1 : 0;
                    case enumType::Number:
                        return (UINT64) dValue;
                    case enumType::Int64:
                    case enumType::Uint64:
                        return uint64Value;
                    default:
                        assert(false); //unexpected error
                        break;
                }
                return 0;
            }

            inline JArray<TChar>& AsArray() const {
                assert(type == enumType::Array);
                return (*arrValue);
            }

            inline JObject<TChar>& AsObject() const {
                assert(type == enumType::Object);
                return (*objValue);
            }

            JValue& operator[](size_t index) const {
                assert(type == enumType::Array);
                return AsArray()[index];
            }

            JValue& operator[](const JString& key) const {
                assert(type == enumType::Object);
                return AsObject()[key];
            }

            JValue& operator[](JString&& key) const {
                assert(type == enumType::Object);
                return AsObject()[key];
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

            JValue* Child(const JString& name) const noexcept {
                if (type != enumType::Object) return nullptr;
                auto it = objValue->find(name);
                if (it != objValue->end()) return &(it->second);
                return nullptr;
            }

            std::vector<JString> ObjKeys() const {
                std::vector<JString> keys;
                if (type == enumType::Object) {
                    for (auto it = objValue->cbegin(), end = objValue->cend(); it != end; ++it) {
                        keys.push_back(it->first);
                    }
                }
                return std::move(keys);
            }

            JString Stringify(bool pretty = true, unsigned char precision = DEFAULT_DOUBLE_PRECISION, unsigned int indent_chars = DEFAULT_INDENT_CHARS) const {
                JString ret_str;
                unsigned int indent = 0;
                Stringify(ret_str, indent, precision, pretty, indent_chars);
                return std::move(ret_str);
            }

            JValue& operator=(JArray<TChar>&& arr) {
                if (type == enumType::Array) {
                    arrValue->swap(arr);
                } else {
                    Clean();
                    type = enumType::Array;
                    arrValue = new JArray<TChar>;
                    arrValue->swap(arr);
                }
                return *this;
            }

            JValue& operator=(JObject<TChar>&& obj) {
                if (type == enumType::Object) {
                    objValue->swap(obj);
                } else {
                    Clean();
                    type = enumType::Object;
                    objValue = new JObject<TChar>;
                    objValue->swap(obj);
                }
                return *this;
            }

            JValue& operator=(JString&& src) {
                if (type == enumType::String) {
                    strValue->swap(src);
                } else {
                    Clean();
                    type = enumType::String;
                    strValue = new JString;
                    strValue->swap(src);
                }
                Escape(*strValue);
                return *this;
            }

            JValue& operator=(JValue&& src) noexcept {
                if (this == &src) return *this;
                enumType t = type;
                UINT64 int64 = uint64Value;
                type = src.type;
                uint64Value = src.uint64Value;
                src.type = t;
                src.uint64Value = int64;
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
                type = (n < 0) ? enumType::Int64 : enumType::Uint64;
                int64Value = n;
                return *this;
            }

            JValue& operator=(unsigned int n) {
                Clean();
                type = enumType::Uint64;
                uint64Value = n;
                return *this;
            }

            JValue& operator=(INT64 n) {
                Clean();
                type = (n < 0) ? enumType::Int64 : enumType::Uint64;
                int64Value = n;
                return *this;
            }

            JValue& operator=(UINT64 n) {
                Clean();
                type = enumType::Uint64;
                uint64Value = n;
                return *this;
            }

            JValue& operator=(const TChar* str) {
                Clean();
                if (str) {
                    type = enumType::String;
                    strValue = new JString(str);
                    Escape(*strValue);
                } else {
                    type = enumType::Null;
                }
                return *this;
            }

            JValue& operator=(const JString& str) {
                Clean();
                type = enumType::String;
                strValue = new JString(str);
                Escape(*strValue);
                return *this;
            }

            JValue& operator=(const JArray<TChar>& arr) {
                Clean();
                type = enumType::Array;
                arrValue = new JArray<TChar>(arr);
                return *this;
            }

            JValue& operator=(const JObject<TChar>& obj) {
                Clean();
                type = enumType::Object;
                objValue = new JObject<TChar>(obj);
                return *this;
            }

            JValue& operator=(const JValue& src) {
                if (this == &src) return *this;
                Clean();
                type = src.type;
                switch (type) {
                    case enumType::String:
                        strValue = new JString(*src.strValue);
                        break;
                    case enumType::Bool:
                        bValue = src.bValue;
                        break;
                    case enumType::Int64:
                        int64Value = src.int64Value;
                        break;
                    case enumType::Uint64:
                        uint64Value = src.uint64Value;
                        break;
                    case enumType::Number:
                        dValue = src.dValue;
                        break;
                    case enumType::Array:
                        arrValue = new JArray<TChar>(*src.arrValue);
                        break;
                    case enumType::Object:
                        objValue = new JObject<TChar>(*src.objValue);
                        break;
                    default: //null
                        break;
                }
                return *this;
            }

            bool operator==(const JValue& jv) const noexcept {
                if (type != jv.type) return false;
                switch (type) {
                    case enumType::String:
                        return (*strValue == *src.strValue);
                    case enumType::Bool:
                        return (bValue == src.bValue);
                    case enumType::Int64:
                        return (int64Value == src.int64Value);
                    case enumType::Uint64:
                        return (uint64Value == src.uint64Value);
                    case enumType::Number:
                        return (dValue == src.dValue);
                    case enumType::Array:
                        return (*arrValue == *src.arrValue);
                    case enumType::Object:
                        return (*objValue == *src.objValue);
                    default: //null
                        break;
                }
                return true;
            }

            bool operator!=(const JValue& jv) const noexcept {
                return (!(*this == jv));
            }

            static JValue* Parse(const TChar* data) {
                if (!data) return nullptr;
                if (!SkipWhitespace(data)) return nullptr;
                auto value = ParseStr(data);
                if (value == nullptr) return nullptr;
                if (SkipWhitespace(data)) {
                    delete value;
                    return nullptr;
                }
                return value;
            }

        private:

            static bool SkipWhitespace(const TChar*& data) {
                while (*data == ' ' || *data == '\n' || *data == '\r' || *data == '\t') {
                    ++data;
                }
                return *data;
            }

            static double ParseDecimal(const TChar*& data) {
                double decimal = 0.0, factor = 0.1;
                while (*data >= '0' && *data <= '9') {
                    int diff = (*data++ -'0');
                    decimal += diff * factor;
                    factor *= 0.1;
                }
                return decimal;
            }

            static bool ExtractString(const TChar*& data, std::basic_string<TChar>& str) {
                const TChar* start = data;
                while (*data) {
                    char next = *data;
                    if (next == '\\') {
                        ++data;
                        switch (*data) {
                            case '"': case '\\': case 'b': case 'f':
                            case 'n': case 'r': case 't': case '/':
                                next = *data;
                                break;
                            case 'u':
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
                    } else if (next > 0 && next < ' ' && next != '\t') {
                        return false;
                    }
                    ++data;
                }
                return false;
            }

            static JValue* ParseStr(const TChar*& data) {
                if (*data == '"') {
                    JString str;
                    if (!ExtractString(++data, str)) return nullptr;
                    return new JValue(std::move(str), false);
                } else if (data[0] == 'f' && data[1] == 'a' && data[2] == 'l' && data[3] == 's' && data[4] == 'e') {
                    data += 5;
                    return new JValue(false);
                } else if (data[0] == 't' && data[1] == 'r' && data[2] == 'u' && data[3] == 'e') {
                    data += 4;
                    return new JValue(true);
                } else if (data[0] == 'n' && data[1] == 'u' && data[2] == 'l' && data[3] == 'l') {
                    data += 4;
                    return new JValue();
                } else if (*data == '-' || (*data >= '0' && *data <= '9')) {
                    bool neg = *data == '-';
                    if (neg) ++data;
                    INT64 int64 = 0;
                    UINT64 uint64 = 0;
                    if (*data == '0') {
                        ++data;
                    } else if (*data >= '1' && *data <= '9') {
                        if (neg) {
                            int64 = atoll(data, data);
                        } else {
                            uint64 = atoull(data, data);
                        }
                    } else {
                        return nullptr;
                    }
                    if (*data != '.' && *data != 'e' && *data != 'E') {
                        if (neg) {
                            return new JValue(-int64);
                        } else {
                            return new JValue(uint64);
                        }
                    }
                    double number = neg ? (double) int64 : (double) uint64;
                    if (*data == '.') {
                        ++data;
                        if (!(*data >= '0' && *data <= '9')) return nullptr;
                        number += ParseDecimal(data);
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
                    JObject<TChar> object;
                    ++data;
                    while (*data) {
                        if (!SkipWhitespace(data)) return nullptr;
                        if (object.size() == 0 && *data == '}') {
                            ++data;
                            return new JValue(object);
                        }
                        if (*data != '"') return nullptr;
                        JString name;
                        if (!ExtractString(++data, name)) return nullptr;
                        if (!SkipWhitespace(data)) return nullptr;
                        if (*data++ != ':') return nullptr;
                        if (!SkipWhitespace(data)) return nullptr;
                        JValue* value = ParseStr(data);
                        if (value == nullptr) return nullptr;
                        object[std::move(name)] = std::move(*value);
                        delete value;
                        if (!SkipWhitespace(data)) return nullptr;
                        if (*data == '}') {
                            ++data;
                            return new JValue(std::move(object));
                        }
                        if (*data != ',') return nullptr;
                        ++data;
                    }
                    return nullptr;
                } else if (*data == '[') {
                    JArray<TChar> array;
                    ++data;
                    while (*data) {
                        if (!SkipWhitespace(data)) return nullptr;
                        if (array.size() == 0 && *data == ']') {
                            ++data;
                            return new JValue(array);
                        }
                        JValue* value = ParseStr(data);
                        if (value == nullptr) return nullptr;
                        array.push_back(std::move(*value));
                        delete value;
                        if (!SkipWhitespace(data)) return nullptr;
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

            void Stringify(JString& ret_str, unsigned int& indent, unsigned char precision, bool pretty, unsigned int indent_chars) const {
                switch (type) {
                    case enumType::Int64:
                    {
                        TChar temp[22];
                        unsigned char chars = (unsigned char) (sizeof (temp) / sizeof (TChar));
                        auto head = ToString(int64Value, temp, chars);
                        ret_str.append(head, (size_t) chars);
                    }
                        break;
                    case enumType::Uint64:
                    {
                        TChar temp[22];
                        unsigned char chars = (unsigned char) (sizeof (temp) / sizeof (TChar));
                        auto head = ToString(uint64Value, temp, chars);
                        ret_str.append(head, (size_t) chars);
                    }
                        break;
                    case enumType::String:
                        ret_str.push_back('"');
                        ret_str.append(*strValue);
                        ret_str.push_back('"');
                        break;
                    case enumType::Bool:
                        if (bValue) {
                            ret_str.append(JTrue.c_str(), (size_t) 4);
                        } else {
                            ret_str.append(JFalse.c_str(), (size_t) 5);
                        }
                        break;
                    case enumType::Number:
                        if (isinf(dValue) || isnan(dValue)) {
                            ret_str.append(JNull.c_str(), (size_t) 4);
                        } else {
                            TChar str[64]; //don't give me a too long precision!!!
                            unsigned char chars = (unsigned char) (sizeof (str) / sizeof (TChar));
                            auto head = ToString(dValue, str, chars, precision);
                            ret_str.append(head, (size_t) chars);
                        }
                        break;
                    case enumType::Array:
                    {
                        ret_str.push_back('[');
                        if (pretty) ret_str.push_back('\n');
                        ++indent;
                        auto iter = arrValue->cbegin(), end = arrValue->cend();
                        while (iter != end) {
                            if (pretty) ret_str.append((size_t) indent * indent_chars, ' ');
                            iter->Stringify(ret_str, indent, precision, pretty, indent_chars);
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
                        auto iter = objValue->cbegin(), end = objValue->cend();
                        while (iter != end) {
                            if (pretty) ret_str.append((size_t) indent * indent_chars, ' ');
                            ret_str.push_back('"');
                            ret_str.append(iter->first);
                            ret_str.append(JMVSep.c_str(), (size_t) 2);
                            iter->second.Stringify(ret_str, indent, precision, pretty, indent_chars);
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
                        ret_str.append(JNull.c_str(), (size_t) 4);
                        break;
                }
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
                UINT64 uint64Value;
                double dValue;
                JString* strValue;
                JArray<TChar>* arrValue;
                JObject<TChar>* objValue;
            };

            static JString JFalse;
            static JString JTrue;
            static JString JNull;
            static JString JMVSep;
        };

        template <typename TChar>
        typename JValue<TChar>::JString JValue<TChar>::JFalse({'f', 'a', 'l', 's', 'e'});
        template <typename TChar>
        typename JValue<TChar>::JString JValue<TChar>::JTrue({'t', 'r', 'u', 'e'});
        template <typename TChar>
        typename JValue<TChar>::JString JValue<TChar>::JNull({'n', 'u', 'l', 'l'});
        template <typename TChar>
        typename JValue<TChar>::JString JValue<TChar>::JMVSep({'"', ':'});

        template <typename TChar>
        JValue<TChar>* Parse(const TChar* data) {
            return JValue<TChar>::Parse(data);
        }

        static JValue<char>* ParseFromFile(const char* filePath, int &errCode) {
            errCode = 0;
#ifdef WIN32_64
            FILE* f = nullptr;
            errCode = ::fopen_s(&f, filePath, "r");
            if (!f) return nullptr;
            std::shared_ptr<FILE> fp(f, [](FILE * f) {
                if (f) {
                    ::fclose(f);
                }
            });
#else
            std::shared_ptr<FILE> fp(fopen(filePath, "r"), [](FILE * f) {
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
            SPA::CScopeUQueue sb(SPA::GetOS(), SPA::IsBigEndian(), (unsigned int) size + sizeof (wchar_t));
            auto res = ::fread((char*) sb->GetBuffer(), 1, (size_t) size, fp.get());
            fp.reset();
            sb->SetSize((unsigned int) res);
            sb->SetNull();
            const char* json = (const char*) sb->GetBuffer();
            if (res >= 3) {
                unsigned char byte_order_mark[] = {0xef, 0xbb, 0xbf}; //UTF8 BOM
                if (::memcmp(byte_order_mark, json, sizeof (byte_order_mark)) == 0) json += 3;
            }
            return Parse<char>(json);
        }
    } //namespace JSON
} //namespace SPA

#endif
