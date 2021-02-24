#ifndef _SOCKETPRO_JSON_VALUE_H_
#define _SOCKETPRO_JSON_VALUE_H_

#include "commutil.h"
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
                str.clear();
                while (*data != 0) {
                    char next = *data;
                    if (next == '\\') {
                        ++data;
                        switch (*data) {
                            case '"': next = '"';
                                break;
                            case '\\': next = '\\';
                                break;
                            case '/': next = '/';
                                break;
                            case 'b': next = '\b';
                                break;
                            case 'f': next = '\f';
                                break;
                            case 'n': next = '\n';
                                break;
                            case 'r': next = '\r';
                                break;
                            case 't': next = '\t';
                                break;
                            case 'u':
                                if (!AtLeastChars(data, 5)) {
                                    return false;
                                }
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
                        ++data;
                        return true;
                    } else if (next < ' ' && next != '\t') {
                        return false;
                    }
                    str += next;
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

        class JValue {
        public:

            JValue() noexcept : type(enumType::Null) {
            }

            JValue(const char* str) {
                if (str) {
                    type = enumType::String;
                    strValue = new std::string(str);
                } else {
                    type = enumType::Null;
                }
            }

            JValue(const std::string& str) : type(enumType::String), strValue(new std::string(str)) {
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
                    case enumType::Null:
                        break;
                    default:
                        assert(false);
                        break;
                }
            }

            JValue(JArray&& arr) : type(enumType::Array), arrValue(new JArray) {
                arrValue->swap(arr);
            }

            JValue(JObject&& obj) : type(enumType::Object), objValue(new JObject) {
                objValue->swap(obj);
            }

            JValue(std::string&& src) : type(enumType::String), strValue(new std::string) {
                strValue->swap(src);
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

            inline std::string& AsString() {
                assert(type == enumType::String);
                return (*strValue);
            }

            bool AsBool() const noexcept {
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
                    default:
                        assert(false); //unexpected error
                        break;
                }
                return false;
            }

            double AsNumber() const noexcept {
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

            INT64 AsInt64() const noexcept {
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

            JArray& AsArray() const {
                assert(type == enumType::Array);
                return (*arrValue);
            }

            JObject& AsObject() const {
                assert(type == enumType::Object);
                return (*objValue);
            }

            std::size_t Size() const noexcept {
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
                if (index < arrValue->size()) {
                    return &arrValue->at(index);
                }
                return nullptr;
            }

            JValue* Child(const char* name) const noexcept {
                if (type != enumType::Object) return nullptr;
                JObject::iterator it = objValue->find(name);
                if (it != objValue->end()) {
                    return &(it->second);
                }
                return nullptr;
            }

            std::vector<std::string> ObjectKeys() const {
                std::vector<std::string> keys;
                if (type == enumType::Object) {
                    for (auto it = objValue->cbegin(), end = objValue->cend(); it != end; ++it) {
                        keys.push_back(it->first);
                    }
                }
                return std::move(keys);
            }

            std::string Stringify(bool pretty = true, unsigned int indent_chars = DEFAULT_INDENT_CHARS) const {
                unsigned int indent = 0;
                return Stringify(indent, pretty, indent_chars);
            }

            std::string Stringify(unsigned int& indent, bool pretty = true, unsigned int indent_chars = DEFAULT_INDENT_CHARS) const {
                std::string ret_string;
                switch (type) {
                    case enumType::Null:
                        ret_string = "null";
                        break;
                    case enumType::Int64:
                        ret_string = std::to_string(int64Value);
                        break;
                    case enumType::String:
                        ret_string = Stringify(*strValue);
                        break;
                    case enumType::Bool:
                        ret_string = bValue ? "true" : "false";
                        break;
                    case enumType::Number:
                    {
                        if (isinf(dValue) || isnan(dValue)) {
                            ret_string = "null";
                        } else {
                            std::stringstream ss;
                            ss.precision(17);
                            ss << dValue;
                            ret_string = ss.str();
                        }
                        break;
                    }
                    case enumType::Array:
                    {
                        ret_string.push_back('[');
                        if (pretty) {
                            ret_string.push_back('\n');
                        }
                        ++indent;
                        JArray::const_iterator iter = arrValue->begin();
                        while (iter != arrValue->end()) {
                            if (pretty) {
                                ret_string += std::string((size_t) indent * indent_chars, ' ');
                            }
                            ret_string += iter->Stringify(indent, pretty, indent_chars);
                            if (++iter != arrValue->end()) {
                                ret_string.push_back(',');
                                if (pretty) {
                                    ret_string.push_back('\n');
                                }
                            }
                        }
                        --indent;
                        if (pretty) {
                            ret_string.push_back('\n');
                            ret_string += std::string((size_t) indent * indent_chars, ' ');
                        }
                        ret_string.push_back(']');
                        break;
                    }
                    case enumType::Object:
                    {
                        ret_string.push_back('{');
                        if (pretty) {
                            ret_string.push_back('\n');
                        }
                        ++indent;
                        JObject::const_iterator iter = objValue->begin();
                        while (iter != objValue->end()) {
                            if (pretty) {
                                ret_string += std::string((size_t) indent * indent_chars, ' ');
                            }
                            ret_string += Stringify(iter->first);
                            ret_string.push_back(':');
                            ret_string += iter->second.Stringify(indent, pretty, indent_chars);

                            // Not at the end - add a separator
                            if (++iter != objValue->end()) {
                                ret_string.push_back(',');
                                if (pretty) {
                                    ret_string.push_back('\n');
                                }
                            }
                        }
                        --indent;
                        if (pretty) {
                            ret_string.push_back('\n');
                            ret_string += std::string((size_t) indent * indent_chars, ' ');
                        }
                        ret_string.push_back('}');
                        break;
                    }
                    default:
                        assert(false);
                        break;
                }
                return std::move(ret_string);
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
                return *this;
            }

            JValue& operator=(JValue&& src) noexcept {
                if (this == &src) {
                    return *this;
                }
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
                } else {
                    type = enumType::Null;
                }
                return *this;
            }

            JValue& operator=(const std::string& str) {
                Clean();
                type = enumType::String;
                strValue = new std::string(str);
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
                if (this == &src) {
                    return *this;
                }
                Clean();
                type = src.type;
                switch (type) {
                    case enumType::Null:
                        break;
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
                    default:
                        assert(false);
                        break;
                }
                return *this;
            }

        private:

            static std::string Stringify(const std::string& str) {
                std::string str_out("\"");
                std::string::const_iterator iter = str.cbegin();
                while (iter != str.cend()) {
                    char chr = *iter;
                    if (chr == '"' || chr == '\\' || chr == '/') {
                        str_out += '\\';
                        str_out += chr;
                    } else if (chr == '\b') {
                        str_out += "\\b";
                    } else if (chr == '\f') {
                        str_out += "\\f";
                    } else if (chr == '\n') {
                        str_out += "\\n";
                    } else if (chr == '\r') {
                        str_out += "\\r";
                    } else if (chr == '\t') {
                        str_out += "\\t";
                    } else if (chr < ' ' || chr > 126) {
                        str_out += "\\u";
                        for (int i = 0; i < 4; i++) {
                            int value = (chr >> 12) & 0xf;
                            if (value >= 0 && value <= 9)
                                str_out += (char) ('0' + value);
                            else if (value >= 10 && value <= 15)
                                str_out += (char) ('A' + (value - 10));
                            chr <<= 4;
                        }
                    } else {
                        str_out += chr;
                    }
                    ++iter;
                }
                str_out += '"';
                return str_out;
            }

            static JValue* Parse(const char*& data) {
                if (*data == '"') {
                    std::string str;
                    if (!Internal::ExtractString(++data, str)) {
                        return nullptr;
                    } else {
                        return new JValue(str);
                    }
                } else if ((Internal::AtLeastChars(data, 4) && ::strncmp(data, "true", 4) == 0) || (Internal::AtLeastChars(data, 5) && ::strncmp(data, "false", 5) == 0)) {
                    bool value = (::strncmp(data, "true", 4) == 0);
                    data += value ? 4 : 5;
                    return new JValue(value);
                } else if (Internal::AtLeastChars(data, 4) && ::strncmp(data, "null", 4) == 0) {
                    data += 4;
                    return new JValue();
                } else if (*data == '-' || (*data >= '0' && *data <= '9')) {
                    bool neg = *data == '-';
                    if (neg) {
                        ++data;
                    }
                    INT64 int64 = 0;
                    if (*data == '0') {
                        ++data;
                    } else if (*data >= '1' && *data <= '9') {
                        int64 = atoll(data, data);
                    } else {
                        return nullptr;
                    }
                    if (*data != '.' && *data != 'e' && *data != 'E') {
                        return new JValue(neg ? -int64 : int64);
                    }
                    double number = int64;
                    if (*data == '.') {
                        ++data;
                        if (!(*data >= '0' && *data <= '9')) {
                            return nullptr;
                        }
                        number += Internal::ParseDecimal(data);
                    }
                    if (*data == 'E' || *data == 'e') {
                        ++data;
                        bool neg_expo = false;
                        if (*data == '-' || *data == '+') {
                            neg_expo = *data == '-';
                            ++data;
                        }
                        if (!(*data >= '0' && *data <= '9')) {
                            return nullptr;
                        }
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
                        if (!Internal::SkipWhitespace(data)) {
                            return nullptr;
                        }
                        if (object.size() == 0 && *data == '}') {
                            ++data;
                            return new JValue(object);
                        }
                        if (*data != '"') {
                            return nullptr;
                        }
                        std::string name;
                        if (!Internal::ExtractString(++data, name)) {
                            return nullptr;
                        }
                        if (!Internal::SkipWhitespace(data)) {
                            return nullptr;
                        }
                        if (*data++ != ':') {
                            return nullptr;
                        }
                        if (!Internal::SkipWhitespace(data)) {
                            return nullptr;
                        }
                        JValue* value = Parse(data);
                        if (value == nullptr) {
                            return nullptr;
                        }
                        object[name] = std::move(*value);
                        delete value;
                        if (!Internal::SkipWhitespace(data)) {
                            return nullptr;
                        }
                        if (*data == '}') {
                            ++data;
                            return new JValue(std::move(object));
                        }
                        if (*data != ',') {
                            return nullptr;
                        }
                        ++data;
                    }
                    return nullptr;
                } else if (*data == '[') {
                    JArray array;
                    ++data;
                    while (*data) {
                        if (!Internal::SkipWhitespace(data)) {
                            return nullptr;
                        }
                        if (array.size() == 0 && *data == ']') {
                            ++data;
                            return new JValue(array);
                        }
                        JValue* value = Parse(data);
                        if (value == nullptr) {
                            return nullptr;
                        }
                        array.push_back(std::move(*value));
                        delete value;
                        if (!Internal::SkipWhitespace(data)) {
                            return nullptr;
                        }
                        if (*data == ']') {
                            ++data;
                            return new JValue(std::move(array));
                        }
                        if (*data != ',') {
                            return nullptr;
                        }
                        ++data;
                    }
                    return nullptr;
                }
                return nullptr;
            }

            void Clean() {
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
            if (!data) {
                return nullptr;
            }
            if (!Internal::SkipWhitespace(data)) {
                return nullptr;
            }
            JValue* value = JValue::Parse(data);
            if (value == nullptr) {
                return nullptr;
            }
            if (Internal::SkipWhitespace(data)) {
                delete value;
                return nullptr;
            }
            return value;
        }
    } //namespace JSON
} //namespace SPA

#endif
