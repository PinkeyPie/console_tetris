#pragma once
#include "stddef.h"
#include "types.h"
#ifdef WIN32
#include "windows.h"
#endif
#include "CollectionGlobal.h"

#define UNINITIALIZED_COLLECTION 0

COLLECTION_EXPORT HANDLE CreateList();
COLLECTION_EXPORT HANDLE CreateVector(EType eVectorType, size_t nElemSize);
COLLECTION_EXPORT BOOL AddElement(HANDLE hCollection, void* pElement, size_t nElemSize);
COLLECTION_EXPORT BOOL AddIntElement(HANDLE hCollection, int nElement);
COLLECTION_EXPORT BOOL AddLongElement(HANDLE hCollection, long nElement);
COLLECTION_EXPORT BOOL AddFloatElement(HANDLE hCollection, float nElement);
COLLECTION_EXPORT BOOL AddDoubleElement(HANDLE hCollection, double nElement);
COLLECTION_EXPORT BOOL AddStringElement(HANDLE hCollection, wchar_t* szElement);
COLLECTION_EXPORT void* GetAt(HANDLE hCollection, size_t nPosition);
COLLECTION_EXPORT int GetIntAt(HANDLE hCollection, size_t nPosition);
COLLECTION_EXPORT wchar_t* GetStringAt(HANDLE hCollection, size_t nPosition);
COLLECTION_EXPORT long GetLongAt(HANDLE hCollection, size_t nPosition);
COLLECTION_EXPORT float GetFloatAt(HANDLE hCollection, size_t nPosition);
COLLECTION_EXPORT double GetDoubleAt(HANDLE hCollection, size_t nPosition);
COLLECTION_EXPORT BOOL RemoveAt(HANDLE hCollection, size_t nPosition);
COLLECTION_EXPORT BOOL DeleteList(HANDLE hCollection);
COLLECTION_EXPORT size_t Size(HANDLE hCollection);
COLLECTION_EXPORT BOOL SetAt(HANDLE hCollection, size_t nPosition, void* pValue, EType eType);
COLLECTION_EXPORT BOOL SetInt(HANDLE hCollection, size_t nPosition, int nValue);
COLLECTION_EXPORT BOOL SetLong(HANDLE hCollection, size_t nPosition, long nValue);
COLLECTION_EXPORT BOOL SetDouble(HANDLE hCollection, size_t nPosition, double nValue);
COLLECTION_EXPORT BOOL SetFloat(HANDLE hCollection, size_t nPosition, float nValue);
COLLECTION_EXPORT BOOL SetString(HANDLE hCollection, size_t nPosition, wchar_t* nValue);
COLLECTION_EXPORT BOOL InsertAt(HANDLE hCollection, size_t nPosition, void* pValue, EType eType);
COLLECTION_EXPORT BOOL InsertIntAt(HANDLE hCollection, size_t nPosition, int nValue);
COLLECTION_EXPORT BOOL InsertLongAt(HANDLE hCollection, size_t nPosition, long nValue);
COLLECTION_EXPORT BOOL InsertDoubleAt(HANDLE hCollection, size_t nPosition, double nValue);
COLLECTION_EXPORT BOOL InsertFloatAt(HANDLE hCollection, size_t nPosition, float nValue);
COLLECTION_EXPORT BOOL InsertStringAt(HANDLE hCollection, size_t nPosition, wchar_t* szValue);
COLLECTION_EXPORT EType GetType(HANDLE hCollection, size_t nPosition);
COLLECTION_EXPORT BOOL SetDestroyFunction(HANDLE hCollection, Destructor);
COLLECTION_EXPORT void FreeCollections();