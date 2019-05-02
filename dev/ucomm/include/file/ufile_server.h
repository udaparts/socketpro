
#pragma once

#include "../spa_module.h"
#include "ufile.h"

#ifndef _U_FILE_MODULE_IMPLEMENTATION_BASIC_HEADER_H_
#define _U_FILE_MODULE_IMPLEMENTATION_BASIC_HEADER_H_

#ifdef __cplusplus
extern "C" {
#endif

    void WINAPI SetRootDirectory(const wchar_t *pathRoot);

#ifdef __cplusplus
}
#endif

typedef void (WINAPI *PSetRootDirectory)(const wchar_t *pathRoot);

#endif