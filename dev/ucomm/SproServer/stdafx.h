// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>
#include "../include/definebase.h"
#ifndef WIN32_64
//#include<boost/uuid/uuid.hpp>
#endif
#include "../include/membuffer.h"
#include "../include/userver.h"
#ifdef WIN32_64
#include "targetver.h"
#include <tchar.h>
#endif
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <map>