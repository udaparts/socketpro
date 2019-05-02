
#pragma once

#include "../include/membuffer.h"
#include "uvariant2rj.h"

namespace SPA {

    typedef rapidjson::Writer<CUQueue> UJsonWriter;

    UJsonValue MakeJsonValue(const char *str, UMemoryPoolAllocator &allocator);
    UJsonValue MakeJsonValue(const wchar_t *str, UMemoryPoolAllocator &allocator);
    UJsonValue MakeJsonValue(const UVariant &vtData, UMemoryPoolAllocator &allocator);

}; //namespace SPA



