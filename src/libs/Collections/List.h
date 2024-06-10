#pragma once
#include "types.h"
#include "stddef.h"
#ifdef WIN32
#include "windows.h"
#endif

HANDLE ListCreate();
BOOL ListAddElement(HANDLE hList, void* pElement, EType eType);
BOOL ListAddStruct(HANDLE hList, void* pElem, Destructor destructor);
void* ListGetAt(HANDLE hList, size_t nPosition, EType* eType);
BOOL ListRemoveAt(HANDLE hList, size_t nPosition);
BOOL ListDelete(HANDLE hList);
size_t ListSize(HANDLE hList);
BOOL ListSet(HANDLE hList, size_t nPosition, void* pValue, EType eType);
BOOL ListSetStruct(HANDLE hList, size_t position, void* pValue, Destructor destructor);
BOOL ListInsertAt(HANDLE hList, size_t nPosition, void* pValue, EType eType);
BOOL ListInsertAtStruct(HANDLE hList, size_t nPosition, void* pValue, Destructor destructor);
EType ListGetType(HANDLE hList, size_t nPosition);
BOOL ListSetDestroyFunc(HANDLE hList, Destructor destructor);