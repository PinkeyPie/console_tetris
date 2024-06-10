#include <stdlib.h>
#include "string.h"
#include "Collection.h"
#include "List.h"
#include "Vector.h"
#include "CollectionUtils.h"

typedef struct _AbstractCollection {
    ECollection collectionType;
} AbstractCollection;

HANDLE CreateList() {
    HANDLE dwNewList = 0;
    dwNewList = ListCreate();
    return dwNewList;
}

HANDLE CreateVector(EType eVectorType, size_t nElemSize) {
    HANDLE dwNewVector = 0;
    dwNewVector = VectorCreate(eVectorType, nElemSize);
    return dwNewVector;
}

BOOL AddElement(HANDLE hCollection, void* pElement, size_t nElemSize) {
    BOOL bResult = FALSE;
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return FALSE;
    }
    void* pCollectionElement = malloc(nElemSize);
    memcpy_s(pCollectionElement, nElemSize, pElement, nElemSize);
    if (collection->collectionType == ELinkedList) {
        bResult = ListAddElement(hCollection, pCollectionElement, EStruct);
    }
    else if (collection->collectionType == EVector) {
        bResult = VectorAddElement(hCollection, pCollectionElement);
    }
    if (!bResult || collection->collectionType == EVector) {
        free(pCollectionElement);
    }
    return bResult;
}

#define AddListElement(Type, TypeName)                                      \
BOOL Add##TypeName##Element(HANDLE hCollection, Type nElement) {            \
    AbstractCollection* collection = (AbstractCollection*)hCollection;      \
    if(collection == NULL) {                                                \
        return FALSE;                                                       \
    }                                                                       \
    void* pElement = malloc(sizeof(Type));                                  \
    *(Type*)pElement = nElement;                                            \
    BOOL bResult = FALSE;                                                   \
    if (collection->collectionType == ELinkedList) {                        \
        bResult = ListAddElement(collection, pElement, EInt);               \
    }                                                                       \
    else if (collection->collectionType == EVector) {                       \
        bResult = VectorAddElement(collection, pElement);                   \
    }                                                                       \
    if (!bResult || collection->collectionType == EVector) {                \
        free(pElement);                                                     \
    }                                                                       \
    return bResult;                                                         \
}

AddListElement(int, Int)
AddListElement(long, Long)
AddListElement(float, Float)
AddListElement(double, Double)

BOOL AddStringElement(HANDLE hCollection, wchar_t* szElement) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return FALSE;
    }
    size_t nStringSize = (wcslen(szElement) + 1);
    wchar_t* szNewElem = malloc(nStringSize * sizeof(wchar_t));
    wcscpy_s(szNewElem, nStringSize, szElement);
    void* pElement = (void*)szNewElem;
    BOOL bResult = FALSE;
    if (collection->collectionType == ELinkedList) {
        bResult = ListAddElement(collection, pElement, EString);
    }
    else if (collection->collectionType == EVector) {
        bResult = VectorAddElement(collection, pElement);
    }
    if (!bResult || collection->collectionType == EVector) {
        free(pElement);
    }
    return bResult;
}

void* GetAt(HANDLE hCollection, size_t nPosition) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return NULL;
    }
    if (collection->collectionType == ELinkedList) {
        EType type = EInt;
        return ListGetAt(collection, nPosition, &type);
    }
    else if (collection->collectionType == EVector) {
        return VectorGetAt(collection, nPosition);
    }
    return FALSE;
}

#define GetFunction(Type, TypeName, ETypeName, TypeReturn)              \
Type Get##TypeName##At(HANDLE hCollection, size_t nPosition) {          \
    AbstractCollection* collection = (AbstractCollection*)hCollection;  \
    if(collection == NULL) {                                            \
        return TypeReturn;                                              \
    }                                                                   \
    EType eType = ETypeName;                                            \
    if (collection->collectionType == ELinkedList) {                    \
            void* pValue = ListGetAt(collection, nPosition, &eType);    \
        if (pValue == NULL) {                                           \
            return TypeReturn;                                          \
        }                                                               \
        if (eType == ETypeName) {                                       \
            return *(Type*)pValue;                                      \
        }                                                               \
    }                                                                   \
    else if (collection->collectionType == EVector) {                   \
        Type nValue = 0;                                                \
        if (VectorGetType(collection, 0) == ETypeName) {                \
            void* pElem = VectorGetAt(collection, nPosition);           \
        nValue = *(Type*)(pElem);                                       \
        }                                                               \
        return nValue;                                                  \
    }                                                                   \
    return 0;                                                           \
}

GetFunction(int, Int, EInt, 0)
GetFunction(long, Long, ELong, 0)
GetFunction(float, Float, EFloat, 0.f)
GetFunction(double, Double, EDouble, 0.0)
GetFunction(wchar_t*, String, EString, L"")

BOOL RemoveAt(HANDLE hCollection, size_t nPosition) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return FALSE;
    }
    if (collection->collectionType == ELinkedList) {
        return ListRemoveAt(collection, nPosition);
    }
    else if (collection->collectionType == EVector) {
        return VectorRemoveAt(collection, nPosition);
    }
    return FALSE;
}

BOOL DeleteList(HANDLE hCollection) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return FALSE;
    }
    if (collection->collectionType == ELinkedList) {
        return ListDelete(collection);
    }
    else if (collection->collectionType == EVector) {
        return VectorDelete(collection);
    }
    return FALSE;
}

size_t Size(HANDLE hCollection) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return FALSE;
    }
    if (collection->collectionType == ELinkedList) {
        return ListSize(collection);
    }
    else if (collection->collectionType == EVector) {
        return VectorSize(collection);
    }
    return 0;
}

BOOL Set(HANDLE hCollection, size_t nPosition, void* pValue, EType eType) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return FALSE;
    }
    if (collection->collectionType == ELinkedList) {
        return ListSet(collection, nPosition, pValue, eType);
    }
    else if (collection->collectionType == EVector) {
        return VectorSet(collection, nPosition, pValue);
    }
    return FALSE;
}

#define SetFunction*()

BOOL SetInt(DWORD dwList, size_t nPosition, int nValue) {
    void* pValue = malloc(sizeof(int));
    *(int*)pValue = nValue;
    BOOL bIsSet = FALSE;
    if (dwList & ELinkedList) {
        bIsSet = ListSet(dwList ^ ELinkedList, nPosition, pValue, EInt);
    }
    else if (dwList & EVector) {
        if (VectorGetType(dwList ^ EVector, 0) == EInt) {
            bIsSet = VectorSet(dwList ^ EVector, nPosition, pValue);
        }
    }
    if (!bIsSet || dwList & EVector) {
        free(pValue);
    }
    return bIsSet;
}

BOOL SetLong(DWORD dwList, size_t nPosition, long nValue) {
    void* pValue = malloc(sizeof(long));
    *(long*)pValue = nValue;
    BOOL bIsSet = FALSE;
    if (dwList & ELinkedList) {
        bIsSet = ListSet(dwList ^ ELinkedList, nPosition, pValue, ELong);
    }
    else if (dwList & EVector) {
        if (VectorGetType(dwList ^ EVector, 0) == ELong) {
            bIsSet = VectorSet(dwList ^ EVector, nPosition, pValue);
        }
    }
    if (!bIsSet || dwList & EVector) {
        free(pValue);
    }
    return bIsSet;
}
BOOL SetDouble(DWORD dwList, size_t nPosition, double nValue) {
    void* pValue = malloc(sizeof(double));
    *(double*)pValue = nValue;
    BOOL bIsSet = FALSE;
    if (dwList & ELinkedList) {
        bIsSet = ListSet(dwList ^ ELinkedList, nPosition, pValue, EDouble);
    }
    else if (dwList & EVector) {
        if (VectorGetType(dwList ^ EVector, 0) == EDouble) {
            bIsSet = VectorSet(dwList ^ EVector, nPosition, pValue);
        }
    }
    if (!bIsSet || dwList & EVector) {
        free(pValue);
    }
    return bIsSet;
}

BOOL SetFloat(DWORD dwList, size_t nPosition, float nValue) {
    void* pValue = malloc(sizeof(float));
    *(float*)pValue = nValue;
    BOOL bIsSet = FALSE;
    if (dwList & ELinkedList) {
        bIsSet = ListSet(dwList ^ ELinkedList, nPosition, pValue, EFloat);
    }
    else if (dwList & EVector) {
        if (VectorGetType(dwList ^ EVector, 0) == EFloat) {
            bIsSet = VectorSet(dwList ^ EVector, nPosition, pValue);
        }
    }
    if (!bIsSet || dwList & EVector) {
        free(pValue);
    }
    return bIsSet;
}

BOOL SetString(DWORD dwList, size_t nPosition, wchar_t* szValue) {
    size_t nStrSize = wcslen(szValue) + 1;
    void* pValue = malloc(sizeof(wchar_t) * nStrSize);
#ifdef WIN32
    wcscpy_s(pValue, nStrSize, szValue);
#endif
    BOOL bIsSet = FALSE;
    if (dwList & ELinkedList) {
        bIsSet = ListSet(dwList ^ ELinkedList, nPosition, pValue, EString);
    }
    else if (dwList & EVector) {
        if (VectorGetType(dwList ^ EVector, 0) == EString) {
            bIsSet = VectorSet(dwList ^ EVector, nPosition, pValue);
        }
    }
    if (!bIsSet || dwList & EVector) {
        free(pValue);
    }
    return bIsSet;
}

BOOL InsertAt(DWORD dwList, size_t nPosition, void* pValue, EType eType) {
    if (dwList & ELinkedList) {
        return ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, eType);
    }
    else if (dwList & EVector) {
        return VectorInsertAt(dwList ^ EVector, nPosition, pValue);
    }
    return FALSE;
}

BOOL InsertIntAt(DWORD dwList, size_t nPosition, int nValue) {
    void* pValue = malloc(sizeof(int));
    *(int*)pValue = nValue;
    BOOL bResult = FALSE;
    if (dwList & ELinkedList) {
        bResult = ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, EInt);
    }
    else if (dwList & EVector) {
        if (VectorGetType(dwList ^ EVector, 0) == EInt) {
            bResult = VectorInsertAt(dwList ^ EVector, nPosition, pValue);
        }
    }
    if (!bResult || dwList & EVector) {
        free(pValue);
    }
    return bResult;
}

BOOL InsertLongAt(DWORD dwList, size_t nPosition, long nValue) {
    void* pValue = malloc(sizeof(long));
    *(long*)pValue = nValue;
    BOOL bResult = FALSE;
    if (dwList & ELinkedList) {
        bResult = ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, ELong);
    }
    else if (dwList & EVector) {
        if (VectorGetType(dwList ^ EVector, 0) == ELong) {
            bResult = VectorInsertAt(dwList ^ EVector, nPosition, pValue);
        }
    }
    if (!bResult || dwList & EVector) {
        free(pValue);
    }
    return bResult;
}

BOOL InsertDoubleAt(DWORD dwList, size_t nPosition, double nValue) {
    void* pValue = malloc(sizeof(double));
    *(double*)pValue = nValue;
    BOOL bResult = FALSE;
    if (dwList & ELinkedList) {
        bResult = ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, EDouble);
    }
    else if (dwList & EVector) {
        if (VectorGetType(dwList ^ EVector, 0) == EDouble) {
            bResult = VectorInsertAt(dwList ^ EVector, nPosition, pValue);
        }
    }
    if (!bResult || dwList & EVector) {
        free(pValue);
    }
    return bResult;
}

BOOL InsertFloatAt(DWORD dwList, size_t nPosition, float nValue) {
    void* pValue = malloc(sizeof(float));
    *(float*)pValue = nValue;
    BOOL bResult = FALSE;
    if (dwList & ELinkedList) {
        bResult = ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, EFloat);
    }
    else if (dwList & EVector) {
        if (VectorGetType(dwList ^ EVector, 0) == EFloat) {
            bResult = VectorInsertAt(dwList ^ EVector, nPosition, pValue);
        }
    }
    if (!bResult || dwList & EVector) {
        free(pValue);
    }
    return bResult;
}

BOOL InsertStringAt(DWORD dwList, size_t nPosition, wchar_t* szValue) {
    size_t nSize = wcslen(szValue) + 1;
    void* pValue = malloc(nSize * sizeof(wchar_t));
#ifdef WIN32
    wcscpy_s(pValue, nSize, szValue);
#endif
    BOOL bResult = FALSE;
    if (dwList & ELinkedList) {
        bResult = ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, EString);
    }
    else {
        if (VectorGetType(dwList ^ EVector, 0) == EString) {
            bResult = VectorInsertAt(dwList ^ EVector, nPosition, pValue);
        }
    }
    if (!bResult || dwList & EVector) {
        free(pValue);
    }
    return bResult;
}

EType GetType(DWORD dwList, size_t nPosition) {
    if (dwList & ELinkedList) {
        return ListGetType(dwList ^ ELinkedList, nPosition);
    }
    else if (dwList & EVector) {
        return VectorGetType(dwList ^ EVector, nPosition);
    }
    return EUnknown;
}

BOOL SetDestroyFunction(DWORD dwList, Destructor destructor) {
    if (dwList & ELinkedList) {
        return ListSetDestroyFunc(dwList ^ ELinkedList, destructor);
    }
    else if (dwList & EVector) {
        return VectorSetDestroyFunc(dwList ^ EVector, destructor);
    }
    return FALSE;
}

void FreeCollections() {
    FreeList();
    FreeVectors();
    FreeUnusedStructs();
}