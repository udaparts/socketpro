// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>

#include "../../include/definebase.h"
#ifndef WIN32_64
#include<boost/uuid/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#else
#include "targetver.h"
#include <tchar.h>
#endif
#include <cstdlib>
#include <iostream>

// TODO: reference additional headers your program requires here
