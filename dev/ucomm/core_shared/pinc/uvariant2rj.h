#ifndef	___U_VARIANT_TO_RAPID_JSON___H___
#define ___U_VARIANT_TO_RAPID_JSON___H___

#include <boost/json.hpp>
#include "../../include/spvariant.h"

namespace SPA {
    typedef boost::json::value UJsonValue;
    typedef boost::json::object UJsonObject;
    typedef boost::json::array UJsonArray;

#ifndef WIN32_64
    UJsonValue MakeJsonValue(const char16_t *str);
#endif
    UJsonValue MakeJsonValue(const wchar_t *);
    UJsonValue MakeJsonValue(const UVariant &vtData);


    //! Construct a UJsonValue with primitive types.

    /*! \tparam T primitive types like bool, int, unsigned, int64_t, uint64_t, double and float etc
     */
    template<typename T>
    UJsonValue MakeJsonValue(T t) {
        UJsonValue jv(t);
        return jv;
    }

    template<typename T>
    UJsonValue MakeJsonValue(const T *t, unsigned int count) {
        UJsonArray jv;
        if (!t) {
            count = 0;
        }
        for (unsigned int n = 0; n < count; ++n) {
            jv.push_back(t[n]);
        }
        return std::move(jv);
    }

};

#endif
