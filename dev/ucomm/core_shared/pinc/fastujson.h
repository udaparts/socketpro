
#pragma once

#include "../../include/membuffer.h"
#include "uvariant2rj.h"

namespace SPA {

    CUQueue& operator<<(CUQueue& q, const UJsonValue& jv);

}