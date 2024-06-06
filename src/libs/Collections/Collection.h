#pragma once
#include "stddef.h"
#include "types.h"
#ifdef WIN32
#include "windows.h"
#endif
#include "CollectionGlobal.h"

#define UNINITIALIZED_COLLECTION 0

COLLECTION_EXPORT DWORD _cdecl CreateList();
COLLECTION_EXPORT DWORD CreateVector(EType eVectorType, size_t nElemSize);
COLLECTION_EXPORT BOOL AddElement(DWORD dwList, void* pElement, size_t nElemSize);
COLLECTION_EXPORT BOOL AddIntElement(DWORD dwList, int nElement);
COLLECTION_EXPORT BOOL AddLongElement(DWORD dwList, long nElement);
COLLECTION_EXPORT BOOL AddFloatElement(DWORD dwList, float nElement);
COLLECTION_EXPORT BOOL AddDoubleElement(DWORD dwList, double nElement);
COLLECTION_EXPORT BOOL AddStringElement(DWORD dwList, wchar_t* szElement);
COLLECTION_EXPORT void* GetAt(DWORD dwList, size_t nPosition);
COLLECTION_EXPORT int GetIntAt(DWORD dwList, size_t nPosition);
COLLECTION_EXPORT wchar_t* GetStringAt(DWORD dwList, size_t nPosition);
COLLECTION_EXPORT long GetLongAt(DWORD dwList, size_t nPosition);
COLLECTION_EXPORT float GetFloatAt(DWORD dwList, size_t nPosition);
COLLECTION_EXPORT double GetDoubleAt(DWORD dwList, size_t nPosition);
COLLECTION_EXPORT BOOL RemoveAt(DWORD dwList, size_t nPosition);
COLLECTION_EXPORT BOOL DeleteList(DWORD dwList);
COLLECTION_EXPORT size_t Size(DWORD dwList);
COLLECTION_EXPORT BOOL Set(DWORD dwList, size_t nPosition, void* pValue, EType eType);
COLLECTION_EXPORT BOOL SetInt(DWORD dwList, size_t nPosition, int nValue);
COLLECTION_EXPORT BOOL SetLong(DWORD dwList, size_t nPosition, long nValue);
COLLECTION_EXPORT BOOL SetDouble(DWORD dwList, size_t nPosition, double nValue);
COLLECTION_EXPORT BOOL SetFloat(DWORD dwList, size_t nPosition, float nValue);
COLLECTION_EXPORT BOOL SetString(DWORD dwList, size_t nPosition, wchar_t* nValue);
COLLECTION_EXPORT BOOL InsertAt(DWORD dwList, size_t nPosition, void* pValue, EType eType);
COLLECTION_EXPORT BOOL InsertIntAt(DWORD dwList, size_t nPosition, int nValue);
COLLECTION_EXPORT BOOL InsertLongAt(DWORD dwList, size_t nPosition, long nValue);
COLLECTION_EXPORT BOOL InsertDoubleAt(DWORD dwList, size_t nPosition, double nValue);
COLLECTION_EXPORT BOOL InsertFloatAt(DWORD dwList, size_t nPosition, float nValue);
COLLECTION_EXPORT BOOL InsertStringAt(DWORD dwList, size_t nPosition, wchar_t* szValue);
COLLECTION_EXPORT EType GetType(DWORD dwList, size_t nPosition);
COLLECTION_EXPORT BOOL SetDestroyFunction(DWORD dwList, Destructor);
COLLECTION_EXPORT void _cdecl FreeCollections();