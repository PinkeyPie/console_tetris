#include <stdlib.h>
#include "string.h"
#include "Collection.h"
#include "List.h"
#include "Vector.h"
#include "CollectionUtils.h"

DWORD CreateList() {
    DWORD dwNewList = 0;
    BOOL bHaveUnused = GetUnusedList(&dwNewList);
    if(bHaveUnused) {
        return dwNewList | ELinkedList;
    }
    dwNewList = ListCreate();
    dwNewList |= ELinkedList;
    return dwNewList;
}

DWORD CreateVector(EType eVectorType, size_t nElemSize) {
    DWORD dwNewVector = 0;
    BOOL bHaveUnused = GetUnusedVector(&dwNewVector);
    if(bHaveUnused) {
        return dwNewVector | EVector;
    }
    dwNewVector = VectorCreate(eVectorType, nElemSize);
    dwNewVector |= EVector;
    return dwNewVector;
}

BOOL AddElement(DWORD dwList, void* pElement, size_t nElemSize) {
    BOOL bResult = FALSE;
    void* pCollectionElement = malloc(nElemSize);
    memcpy_s(pCollectionElement, nElemSize, pElement, nElemSize);
    if(dwList & ELinkedList) {
        bResult = ListAddElement(dwList ^ ELinkedList, pCollectionElement, EStruct);
    } else if(dwList & EVector) {
        bResult = VectorAddElement(dwList ^ EVector, pCollectionElement);
    }
    if(!bResult) {
        free(pCollectionElement);
    }
    return bResult;
}

BOOL AddIntElement(DWORD dwList, int nElement) {
    void* pElement = malloc(sizeof(int));
    *(int*)pElement = nElement;
    BOOL bResult = FALSE;
    if(dwList & ELinkedList) {
        bResult = ListAddElement(dwList ^ ELinkedList, pElement, EInt);
    } else if(dwList & EVector) {
        bResult = VectorAddElement(dwList ^ EVector, pElement);
    }
    if(!bResult || dwList & EVector) {
        free(pElement);
    }
    return bResult;
}

BOOL AddLongElement(DWORD dwList, long nElement) {
    void* pElement = malloc(sizeof(long));
    *(long*)pElement = nElement;
    BOOL bResult = FALSE;
    if(dwList ^ ELinkedList) {
        bResult = ListAddElement(dwList ^ ELinkedList, pElement, EInt);
    } else if(dwList ^ EVector) {
        bResult = VectorAddElement(dwList ^ EVector, pElement);
    }
    if(!bResult || dwList & EVector) {
        free(pElement);
    }
    return bResult;
}

BOOL AddFloatElement(DWORD dwList, float nElement) {
    void* pElement = malloc(sizeof(float));
    *(float*)pElement = nElement;
    BOOL bResult = FALSE;
    if(dwList ^ ELinkedList) {
        bResult = ListAddElement(dwList ^ ELinkedList, pElement, EFloat);
    } else if(dwList ^ EVector) {
        bResult = VectorAddElement(dwList ^ EVector, pElement);
    }
    if(!bResult || dwList & EVector) {
        free(pElement);
    }
    return bResult;
}

BOOL AddDoubleElement(DWORD dwList, double nElement) {
    void* pElement = malloc(sizeof(double));
    *(double*)pElement = nElement;
    BOOL bResult = FALSE;
    if(dwList ^ ELinkedList) {
        bResult = ListAddElement(dwList ^ ELinkedList, pElement, EDouble);
    } else if(dwList ^ EVector) {
        bResult = VectorAddElement(dwList ^ EVector, pElement);
    }
    if(!bResult || dwList & EVector) {
        free(pElement);
    }
    return bResult;
}

BOOL AddStringElement(DWORD dwList, wchar_t* szElement) {
    size_t nStringSize = (wcslen(szElement) + 1);
    wchar_t* szNewElem = malloc(nStringSize * sizeof(wchar_t));
    wcscpy_s(szNewElem, nStringSize, szElement);
    void* pElement = (void*)szNewElem;
    BOOL bResult = FALSE;
    if(dwList ^ ELinkedList) {
        bResult = ListAddElement(dwList ^ ELinkedList, pElement, EString);
    } else if(dwList ^ EVector) {
        bResult = VectorAddElement(dwList ^ EVector, pElement);
    }
    if(!bResult || dwList & EVector) {
        free(pElement);
    }
    return bResult;
}

void* GetAt(DWORD dwList, size_t nPosition) {
    if(dwList & ELinkedList) {
        EType type = EInt;
        return ListGetAt(dwList ^ ELinkedList, nPosition, &type);
    } else if(dwList & EVector) {
        return VectorGetAt(dwList ^ EVector, nPosition);
    }
    return FALSE;
}

int GetIntAt(DWORD dwList, size_t nPosition) {
    EType eType = EInt;
    if(dwList & ELinkedList) {
        void* pValue = ListGetAt(dwList ^ ELinkedList, nPosition, &eType);
        if(pValue == NULL) {
            return 0;
        }
        if(eType == EInt) {
            return *(int*)pValue;
        }
    } else if(dwList & EVector) {
        int nValue = 0;
        if(VectorGetType(dwList ^ EVector, 0) == EInt) {
            void *pElem = VectorGetAt(dwList ^ EVector, nPosition);
            nValue = *(int *) (pElem);
        }
        return nValue;
    }
    return 0;
}

wchar_t* GetStringAt(DWORD dwList, size_t nPosition) {
    EType eType = EString;
    if(dwList & ELinkedList) {
        void* pValue = ListGetAt(dwList ^ ELinkedList, nPosition, &eType);
        if(pValue == NULL) {
            return L"";
        }
        if(eType == EString) {
            return (wchar_t*)pValue;
        }
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == EString) {
            void *pElem = VectorGetAt(dwList ^ EVector, nPosition);
            return (wchar_t *) pElem;
        }
    }
    return L"";
}

long GetLongAt(DWORD dwList, size_t nPosition) {
    EType eType = ELong;
    if(dwList & ELinkedList) {
        void* pValue = ListGetAt(dwList ^ ELinkedList, nPosition, &eType);
        if(pValue == NULL) {
            return 0;
        }
        if(eType == ELong) {
            return *(long*)pValue;
        }
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == ELong) {
            long nValue = *(long*)VectorGetAt(dwList ^ EVector, nPosition);
            return nValue;
        }
    }
    return 0;
}

float GetFloatAt(DWORD dwList, size_t nPosition) {
    EType eType = EFloat;
    if(dwList & ELinkedList) {
        void* pValue = ListGetAt(dwList ^ ELinkedList, nPosition, &eType);
        if(pValue == NULL) {
            return 0.f;
        }
        if(eType == EFloat) {
            return *(float*)pValue;
        }
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == EFloat) {
            float nValue = *(float*) VectorGetAt(dwList ^ EVector, nPosition);
            return nValue;
        }
    }
    return 0.0f;
}

double GetDoubleAt(DWORD dwList, size_t nPosition) {
    EType eType = EFloat;
    if(dwList & ELinkedList) {
        void* pValue = ListGetAt(dwList ^ ELinkedList, nPosition, &eType);
        if(pValue == NULL) {
            return 0.0;
        }
        if(eType == EFloat) {
            return *(float*)pValue;
        }
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == EDouble) {
            double nValue = *(double*)VectorGetAt(dwList ^ EVector, nPosition);
            return nValue;
        }
    }
    return 0.0;
}

BOOL RemoveAt(DWORD dwList, size_t nPosition) {
    if(dwList & ELinkedList) {
        return ListRemoveAt(dwList ^ ELinkedList, nPosition);
    } else if(dwList & EVector) {
        return VectorRemoveAt(dwList ^ EVector, nPosition);
    }
    return FALSE;
}

BOOL DeleteList(DWORD dwList) {
    if(dwList & ELinkedList) {
        return ListDelete(dwList ^ ELinkedList);
    } else if(dwList & EVector) {
        return VectorDelete(dwList ^ EVector);
    }
    return FALSE;
}

size_t Size(DWORD dwList) {
    if(dwList & ELinkedList) {
        return ListSize(dwList ^ ELinkedList);
    } else if(dwList & EVector) {
        return VectorSize(dwList ^ EVector);
    }
    return 0;
}

BOOL Set(DWORD dwList, size_t nPosition, void* pValue, EType eType) {
    if(dwList & ELinkedList) {
        return ListSet(dwList ^ ELinkedList, nPosition, pValue, eType);
    } else if(dwList & EVector) {
        return VectorSet(dwList ^ EVector, nPosition, pValue);
    }
    return FALSE;
}

BOOL SetInt(DWORD dwList, size_t nPosition, int nValue) {
    void* pValue = malloc(sizeof(int));
    *(int*)pValue = nValue;
    BOOL bIsSet = FALSE;
    if(dwList & ELinkedList) {
        bIsSet = ListSet(dwList ^ ELinkedList, nPosition, pValue, EInt);
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == EInt) {
            bIsSet = VectorSet(dwList ^ EVector, nPosition, pValue);
        }
    }
    if(!bIsSet || dwList & EVector) {
        free(pValue);
    }
    return bIsSet;
}

BOOL SetLong(DWORD dwList, size_t nPosition, long nValue) {
    void* pValue = malloc(sizeof(long));
    *(long*)pValue = nValue;
    BOOL bIsSet = FALSE;
    if(dwList & ELinkedList) {
        bIsSet = ListSet(dwList ^ ELinkedList, nPosition, pValue, ELong);
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == ELong) {
            bIsSet = VectorSet(dwList ^ EVector, nPosition, pValue);
        }
    }
    if(!bIsSet || dwList & EVector) {
        free(pValue);
    }
    return bIsSet;
}
BOOL SetDouble(DWORD dwList, size_t nPosition, double nValue) {
    void* pValue = malloc(sizeof(double));
    *(double*)pValue = nValue;
    BOOL bIsSet = FALSE;
    if(dwList & ELinkedList) {
        bIsSet = ListSet(dwList ^ ELinkedList, nPosition, pValue, EDouble);
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == EDouble) {
            bIsSet = VectorSet(dwList ^ EVector, nPosition, pValue);
        }
    }
    if(!bIsSet || dwList & EVector) {
        free(pValue);
    }
    return bIsSet;
}

BOOL SetFloat(DWORD dwList, size_t nPosition, float nValue) {
    void* pValue = malloc(sizeof(float));
    *(float*)pValue = nValue;
    BOOL bIsSet = FALSE;
    if(dwList & ELinkedList) {
        bIsSet = ListSet(dwList ^ ELinkedList, nPosition, pValue, EFloat);
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == EFloat) {
            bIsSet = VectorSet(dwList ^ EVector, nPosition, pValue);
        }
    }
    if(!bIsSet || dwList & EVector) {
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
    if(dwList & ELinkedList) {
        bIsSet = ListSet(dwList ^ ELinkedList, nPosition, pValue, EString);
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == EString) {
            bIsSet = VectorSet(dwList ^ EVector, nPosition, pValue);
        }
    }
    if(!bIsSet || dwList & EVector) {
        free(pValue);
    }
    return bIsSet;
}

BOOL InsertAt(DWORD dwList, size_t nPosition, void* pValue, EType eType) {
    if(dwList & ELinkedList) {
        return ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, eType);
    } else if(dwList & EVector) {
        return VectorInsertAt(dwList ^ EVector, nPosition, pValue);
    }
    return FALSE;
}

BOOL InsertIntAt(DWORD dwList, size_t nPosition, int nValue) {
    void* pValue = malloc(sizeof(int));
    *(int*)pValue = nValue;
    BOOL bResult = FALSE;
    if(dwList & ELinkedList) {
        bResult = ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, EInt);
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == EInt) {
            bResult = VectorInsertAt(dwList ^ EVector, nPosition, pValue);
        }
    }
    if(!bResult || dwList & EVector) {
        free(pValue);
    }
    return bResult;
}

BOOL InsertLongAt(DWORD dwList, size_t nPosition, long nValue) {
    void* pValue = malloc(sizeof(long));
    *(long*)pValue = nValue;
    BOOL bResult = FALSE;
    if(dwList & ELinkedList) {
        bResult = ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, ELong);
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == ELong) {
            bResult = VectorInsertAt(dwList ^ EVector, nPosition, pValue);
        }
    }
    if(!bResult || dwList & EVector) {
        free(pValue);
    }
    return bResult;
}

BOOL InsertDoubleAt(DWORD dwList, size_t nPosition, double nValue) {
    void* pValue = malloc(sizeof(double));
    *(double*)pValue = nValue;
    BOOL bResult = FALSE;
    if(dwList & ELinkedList) {
        bResult = ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, EDouble);
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == EDouble) {
            bResult = VectorInsertAt(dwList ^ EVector, nPosition, pValue);
        }
    }
    if(!bResult || dwList & EVector) {
        free(pValue);
    }
    return bResult;
}

BOOL InsertFloatAt(DWORD dwList, size_t nPosition, float nValue) {
    void* pValue = malloc(sizeof(float));
    *(float*)pValue = nValue;
    BOOL bResult = FALSE;
    if(dwList & ELinkedList) {
        bResult = ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, EFloat);
    } else if(dwList & EVector) {
        if(VectorGetType(dwList ^ EVector, 0) == EFloat) {
            bResult = VectorInsertAt(dwList ^ EVector, nPosition, pValue);
        }
    }
    if(!bResult || dwList & EVector) {
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
    if(dwList & ELinkedList) {
        bResult = ListInsertAt(dwList ^ ELinkedList, nPosition, pValue, EString);
    } else {
        if(VectorGetType(dwList ^ EVector, 0) == EString) {
            bResult = VectorInsertAt(dwList ^ EVector, nPosition, pValue);
        }
    }
    if(!bResult || dwList & EVector) {
        free(pValue);
    }
    return bResult;
}

EType GetType(DWORD dwList, size_t nPosition) {
    if(dwList & ELinkedList) {
        return ListGetType(dwList ^ ELinkedList, nPosition);
    } else if(dwList & EVector) {
        return VectorGetType(dwList ^ EVector, nPosition);
    }
    return EUnknown;
}

void FreeCollections() {
    FreeList();
    FreeVectors();
    FreeUnusedStructs();
}