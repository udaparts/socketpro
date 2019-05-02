#ifndef	___U_VARIANT_TO_RAPID_JSON___H___
#define ___U_VARIANT_TO_RAPID_JSON___H___

#include "document.h"		// rapidjson's DOM-style API
#include "writer.h"

namespace SPA {

    typedef rapidjson::MemoryPoolAllocator UMemoryPoolAllocator;
    typedef rapidjson::GenericDocument<rapidjson::UTF8<>, UMemoryPoolAllocator> UJsonDocument;
    typedef UJsonDocument::ValueType UJsonValue;

    UJsonValue MakeJsonValue(const char *str, UMemoryPoolAllocator &allocator);
    UJsonValue MakeJsonValue(const wchar_t *str, UMemoryPoolAllocator &allocator);
    UJsonValue MakeJsonValue(const UVariant &vtData, UMemoryPoolAllocator &allocator);


    //! Construct a UJsonValue with primitive types.

    /*! \tparam T primitive types like bool, int, unsigned, int64_t, uint64_t, double and float etc
     */
    template<typename T>
    UJsonValue MakeJsonValue(T t) {
        UJsonValue jv(t);
        return jv;
    }

    template<typename T>
    UJsonValue MakeJsonValue(const T *t, unsigned int count, UMemoryPoolAllocator &allocator) {
        unsigned int n;
        UJsonValue jv;
        jv.SetArray();
        if (t == NULL)
            count = 0;
        for (n = 0; n < count; ++n) {
            jv.PushBack(t[n], allocator);
        }
        return jv;
    }

};

#endif
