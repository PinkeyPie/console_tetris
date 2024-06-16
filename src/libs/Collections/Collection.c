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
#ifdef _MSC_VER
    memcpy_s(pCollectionElement, nElemSize, pElement, nElemSize);
#else
    memcpy(pCollectionElement, pElement, nElemSize);
#endif
    if (collection->collectionType == ELinkedList) {
        bResult = ListAddElement(hCollection, pCollectionElement, EStruct);
    }
    else if (collection->collectionType == EVector) {
        bResult = VectorAddElement(hCollection, pCollectionElement);
    }
    if (!bResult || collection->collectionType == EVector) {
        if(pCollectionElement != NULL) {
            free(pCollectionElement);
        }
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
#ifdef _MSC_VER
    wcscpy_s(szNewElem, nStringSize, szElement);
#else
    wcscpy(szNewElem, szElement);
#endif
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
            return *((Type*)pValue);                                    \
        }                                                               \
    }                                                                   \
    else if (collection->collectionType == EVector) {                   \
        Type nValue = 0;                                                \
        if (VectorGetType(collection, 0) == ETypeName) {                \
            void* pElem = VectorGetAt(collection, nPosition);           \
            nValue = *((Type*)(pElem));                                 \
        }                                                               \
        return nValue;                                                  \
    }                                                                   \
    return 0;                                                           \
}

GetFunction(int, Int, EInt, 0)
GetFunction(long, Long, ELong, 0)
GetFunction(float, Float, EFloat, 0.f)
GetFunction(double, Double, EDouble, 0.0)

wchar_t* GetStringAt(HANDLE hCollection, size_t nPosition) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return L"";
    }
    EType eType = EString;
    if (collection->collectionType == ELinkedList) {
        void* pValue = ListGetAt(collection, nPosition, &eType);
        if (pValue == NULL) {
            return L"";
        }
        if (eType == EString) {
            return (wchar_t*)pValue;
        }
    }
    else if (collection->collectionType == EVector) {
        wchar_t* pValue = L"";
        if (VectorGetType(collection, 0) == EString) {
            void* pElem = VectorGetAt(collection, nPosition);
            pValue = (wchar_t*)(pElem);
        }
        return pValue;
    }
    return 0;
}

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

BOOL SetAt(HANDLE hCollection, size_t nPosition, void* pValue, EType eType) {
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

#define SetFunction(Type, TypeName, EType)                                      \
BOOL Set##TypeName(HANDLE hCollection, size_t nPosition, Type nValue) {         \
    AbstractCollection* collection = (AbstractCollection*)hCollection;          \
    if(collection == NULL) {                                                    \
        return FALSE;                                                           \
    }                                                                           \
    void* pValue = malloc(sizeof(Type));                                        \
    *(Type*)pValue = nValue;                                                    \
    BOOL bIsSet = FALSE;                                                        \
    if (collection->collectionType == ELinkedList) {                            \
        bIsSet = ListSet(collection, nPosition, pValue, EType);                 \
    }                                                                           \
    else if (collection->collectionType == EVector) {                           \
        if (VectorGetType(collection, 0) == EType) {                            \
            bIsSet = VectorSet(collection, nPosition, pValue);                  \
        }                                                                       \
    }                                                                           \
    if (!bIsSet || collection->collectionType == EVector) {                     \
        free(pValue);                                                           \
    }                                                                           \
    return bIsSet;                                                              \
}

SetFunction(int, Int, EInt)
SetFunction(long, Long, ELong)
SetFunction(float, Float, EFloat)
SetFunction(double, Double, EDouble)

BOOL SetString(HANDLE hCollection, size_t nPosition, wchar_t* szValue) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return FALSE;
    }
    size_t nStrSize = wcslen(szValue) + 1;
    void* pValue = malloc(sizeof(wchar_t) * nStrSize);
#ifdef _MSC_VER
    wcscpy_s(pValue, nStrSize, szValue);
#else
    wcscpy(pValue, szValue);
#endif
    BOOL bIsSet = FALSE;
    if (collection->collectionType == ELinkedList) {
        bIsSet = ListSet(collection, nPosition, pValue, EString);
    }
    else if (collection->collectionType == EVector) {
        if (VectorGetType(collection, 0) == EString) {
            bIsSet = VectorSet(collection, nPosition, pValue);
        }
    }
    if (!bIsSet || collection->collectionType == EVector) {
        free(pValue);
    }
    return bIsSet;
}

BOOL InsertAt(HANDLE hCollection, size_t nPosition, void* pValue, EType eType) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return FALSE;
    }
    if (collection->collectionType == ELinkedList) {
        return ListInsertAt(collection, nPosition, pValue, eType);
    }
    else if (collection->collectionType == EVector) {
        return VectorInsertAt(collection, nPosition, pValue);
    }
    return FALSE;
}

#define InsertFunction(Type, TypeName, EType)                                       \
BOOL Insert##TypeName##At(HANDLE hCollection, size_t nPosition, Type nValue) {      \
    AbstractCollection* collection = (AbstractCollection*)hCollection;              \
    if(collection == NULL) {                                                        \
        return FALSE;                                                               \
    }                                                                               \
    void* pValue = malloc(sizeof(Type));                                            \
    *(Type*)pValue = nValue;                                                        \
    BOOL bResult = FALSE;                                                           \
    if (collection->collectionType == ELinkedList) {                                \
        bResult = ListInsertAt(collection, nPosition, pValue, EType);               \
    }                                                                               \
    else if (collection->collectionType == EVector) {                               \
        if (VectorGetType(collection, 0) == EType) {                                \
            bResult = VectorInsertAt(collection, nPosition, pValue);                \
        }                                                                           \
    }                                                                               \
    if (!bResult || collection->collectionType == EVector) {                        \
        free(pValue);                                                               \
    }                                                                               \
    return bResult;                                                                 \
}


InsertFunction(int, Int, EInt)
InsertFunction(long, Long, ELong)
InsertFunction(double, Double, EDouble)
InsertFunction(float, Float, EFloat)

BOOL InsertStringAt(HANDLE hCollection, size_t nPosition, wchar_t* szValue) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return FALSE;
    }
    size_t nSize = wcslen(szValue) + 1;
    void* pValue = malloc(nSize * sizeof(wchar_t));
#ifdef _MSC_VER
    wcscpy_s(pValue, nSize, szValue);
#else
    wcscpy(pValue, szValue);
#endif
    BOOL bResult = FALSE;
    if (collection->collectionType == ELinkedList) {
        bResult = ListInsertAt(collection, nPosition, pValue, EString);
    }
    else if(collection->collectionType == EVector){
        if (VectorGetType(collection, 0) == EString) {
            bResult = VectorInsertAt(collection, nPosition, pValue);
        }
    }
    if (!bResult || collection->collectionType == EVector) {
        free(pValue);
    }
    return bResult;
}

EType GetType(HANDLE hCollection, size_t nPosition) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return EUnknown;
    }
    if (collection->collectionType == ELinkedList) {
        return ListGetType(collection, nPosition);
    }
    else if (collection->collectionType == EVector) {
        return VectorGetType(collection, nPosition);
    }
    return EUnknown;
}

BOOL SetDestroyFunction(HANDLE hCollection, Destructor destructor) {
    AbstractCollection* collection = (AbstractCollection*)hCollection;
    if(collection == NULL) {
        return EUnknown;
    }
    if (collection->collectionType == ELinkedList) {
        return ListSetDestroyFunc(collection, destructor);
    }
    else if (collection->collectionType == EVector) {
        return VectorSetDestroyFunc(collection, destructor);
    }
    return FALSE;
}

void FreeCollections() {
    FreeUnusedStructs();
}