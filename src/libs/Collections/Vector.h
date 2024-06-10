#pragma once
#include "types.h"
#include "stddef.h"
#ifdef WIN32
#include "windows.h"
#endif

HANDLE VectorCreate(EType eType, size_t nElementSize);
BOOL VectorAddElement(HANDLE hVector, void* pElement);
void* VectorGetAt(HANDLE hVector, size_t nPosition);
BOOL VectorRemoveAt(HANDLE dwVector, size_t nPosition);
BOOL VectorDelete(HANDLE hVector);
BOOL VectorSet(HANDLE hVector, size_t nPosition, void* pValue);
size_t VectorSize(HANDLE hVector);
BOOL VectorInsertAt(HANDLE hVector, size_t nPosition, void* pValue);
EType VectorGetType(HANDLE hVector, size_t nPosition);
BOOL VectorSetDestroyFunc(HANDLE hVector, Destructor destructor);