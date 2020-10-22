
#pragma once

#include "../../include/membuffer.h"
#include "uvariant2rj.h"

namespace SPA {

    typedef rapidjson::Writer<CUQueue> UJsonWriter;

#ifndef WIN32_64
    UJsonValue MakeJsonValue(const char16_t *str, UMemoryPoolAllocator &allocator);
#endif

    UJsonValue MakeJsonValue(const char *str, UMemoryPoolAllocator &allocator);
    UJsonValue MakeJsonValue(const wchar_t *str, UMemoryPoolAllocator &allocator);
    UJsonValue MakeJsonValue(const UVariant &vtData, UMemoryPoolAllocator &allocator);

}; //namespace SPA



