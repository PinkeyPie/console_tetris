#pragma once
#include "types.h"
#include "stddef.h"
#ifdef WIN32
#include "windows.h"
#endif

DWORD ListCreate();
BOOL ListAddElement(DWORD dwList, void* pElement, EType eType);
void* ListGetAt(DWORD dwList, size_t nPosition, EType* pEType);
BOOL ListRemoveAt(DWORD dwList, size_t nPosition);
BOOL ListDelete(DWORD dwList);
size_t ListSize(DWORD dwList);
BOOL ListSet(DWORD dwList, size_t nPosition, void* pValue, EType eType);
BOOL ListInsertAt(DWORD dwList, size_t nPosition, void* pValue, EType eType);
EType ListGetType(DWORD dwList, size_t nPosition);
void FreeList();