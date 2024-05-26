#pragma once
#include "types.h"
#include "stddef.h"

DWORD VectorCreate(EType eType, size_t nElementSize);
BOOL VectorAddElement(DWORD dwVector, void* pElement);
void* VectorGetAt(DWORD dwVector, size_t nPosition);
BOOL VectorRemoveAt(DWORD dwVector, size_t nPosition);
BOOL VectorDelete(DWORD dwVector);
BOOL VectorSet(DWORD dwVector, size_t nPosition, void* pValue);
size_t VectorSize(DWORD dwVector);
BOOL VectorInsertAt(DWORD dwVector, size_t nPosition, void* pValue);
EType VectorGetType(DWORD dwVector, size_t nPosition);
void FreeVectors();