#pragma once

#include <stdio.h>

#include "../include/definebase.h"
#ifdef WIN32_64
#include "targetver.h"
#include <tchar.h>
#include "../include/spvariant.h"
#else
#include<boost/uuid/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#endif
#include <cstdlib>
#include <iostream>
