#pragma once
#include "stddef.h"
#include "types.h"

DWORD CreateList();
DWORD CreateVector(EType eVectorType, size_t nElemSize);
BOOL AddElement(DWORD dwList, void* pElement, size_t nElemSize);
BOOL AddIntElement(DWORD dwList, int nElement);
BOOL AddLongElement(DWORD dwList, long nElement);
BOOL AddFloatElement(DWORD dwList, float nElement);
BOOL AddDoubleElement(DWORD dwList, double nElement);
BOOL AddStringElement(DWORD dwList, wchar_t* szElement);
void* GetAt(DWORD dwList, size_t nPosition);
int GetIntAt(DWORD dwList, size_t nPosition);
wchar_t* GetStringAt(DWORD dwList, size_t nPosition);
long GetLongAt(DWORD dwList, size_t nPosition);
float GetFloatAt(DWORD dwList, size_t nPosition);
double GetDoubleAt(DWORD dwList, size_t nPosition);
BOOL RemoveAt(DWORD dwList, size_t nPosition);
BOOL DeleteList(DWORD dwList);
size_t Size(DWORD dwList);
BOOL Set(DWORD dwList, size_t nPosition, void* pValue, EType eType);
BOOL SetInt(DWORD dwList, size_t nPosition, int nValue);
BOOL SetLong(DWORD dwList, size_t nPosition, long nValue);
BOOL SetDouble(DWORD dwList, size_t nPosition, double nValue);
BOOL SetFloat(DWORD dwList, size_t nPosition, float nValue);
BOOL SetString(DWORD dwList, size_t nPosition, wchar_t* nValue);
BOOL InsertAt(DWORD dwList, size_t nPosition, void* pValue, EType eType);
BOOL InsertIntAt(DWORD dwList, size_t nPosition, int nValue);
BOOL InsertLongAt(DWORD dwList, size_t nPosition, long nValue);
BOOL InsertDoubleAt(DWORD dwList, size_t nPosition, double nValue);
BOOL InsertFloatAt(DWORD dwList, size_t nPosition, float nValue);
BOOL InsertStringAt(DWORD dwList, size_t nPosition, wchar_t* szValue);
EType GetType(DWORD dwList, size_t nPosition);
void FreeCollections();