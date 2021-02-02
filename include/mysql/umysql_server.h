
#pragma once

#include "../spa_module.h"
#include "umysql.h"

#ifndef _MYSQL_MODULE_IMPLEMENTATION_BASIC_HEADER_H_
#define _MYSQL_MODULE_IMPLEMENTATION_BASIC_HEADER_H_

namespace SPA {
    namespace ServerSide {
        namespace Mysql {

            //The two following defines are reserved for the future use

            static const unsigned int DISABLE_REMOTE_MYSQL = 0x80000000;
            static const unsigned int DISABLE_EMBEDDED_MYSQL = 0x40000000;

        } //namespace Mysql
    } //namespace ServerSide
} //namespace SPA

#endif
