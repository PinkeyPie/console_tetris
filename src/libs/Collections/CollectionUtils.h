#pragma once
#include "stddef.h"
#include "types.h"
#ifdef WIN32
#include "windows.h"
#endif

BOOL GetUnusedVector(DWORD* pDwUnused);
BOOL GetUnusedList(DWORD* pDwUnused);
void PutUnusedVector(DWORD pDwUnused);
void PutUnusedList(DWORD pDwUnused);
void FreeUnusedStructs();