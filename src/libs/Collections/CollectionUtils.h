#pragma once
#include "stddef.h"
#include "types.h"

BOOL GetUnusedVector(DWORD* pDwUnused);
BOOL GetUnusedList(DWORD* pDwUnused);
void PutUnusedVector(DWORD pDwUnused);
void PutUnusedList(DWORD pDwUnused);
void FreeUnusedStructs();