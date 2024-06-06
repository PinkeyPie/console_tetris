#include "memory.h"
#include "string.h"
#include "Vector.h"
#include "stdlib.h"
#include "CollectionUtils.h"
#include "Collection.h"

typedef struct Vector {
    void* pElementsMemory;
    EType eVectorType;
    size_t nElemSize;
    size_t nVectorSize;
    size_t nCapacity;
} Vector;

static Vector* pVectorsTable = NULL;
static DWORD dwCurrentTableSize = 0;
static DWORD dwCurrentVectorsSize = 0;
static const DWORD dwNewItemsCount = 10;
static size_t nDefaultCapacity = 10;
static BOOL bClear = FALSE;
static Destructor* pDestructors = NULL;

DWORD VectorCreate(EType eType, size_t nElementSize) {
    DWORD dwRetVector = 0;
    if (pVectorsTable == NULL) {
        pVectorsTable = malloc(sizeof(Vector) * dwNewItemsCount);
        if (pVectorsTable == NULL) {
            return UNINITIALIZED_COLLECTION;
        }
        dwCurrentTableSize += dwNewItemsCount;
    }
    if (pDestructors == NULL) {
        pDestructors = malloc(sizeof(Destructor) * dwNewItemsCount);
        for (DWORD i = 0; i < dwNewItemsCount; i++) {
            pDestructors[i] = NULL;
        }
    }
    else if (dwCurrentVectorsSize >= dwCurrentTableSize) {
        Vector* pTempMem = realloc(pVectorsTable, dwCurrentTableSize + dwNewItemsCount);
        if (pTempMem) {
            pVectorsTable = pTempMem;
        }
        else {
            return UNINITIALIZED_COLLECTION;
        }
        Destructor* pTempDestructors = realloc(pDestructors, dwCurrentTableSize + dwNewItemsCount);
        if (pTempDestructors) {
            pDestructors = pTempDestructors;
            for (DWORD i = dwCurrentTableSize; i < dwCurrentTableSize + dwNewItemsCount; i++) {
                pDestructors[i] = NULL;
            }
        }
        dwCurrentTableSize += dwNewItemsCount;
    }
    dwRetVector = dwCurrentVectorsSize++;
    pVectorsTable[dwRetVector].nCapacity = nDefaultCapacity;
    pVectorsTable[dwRetVector].eVectorType = eType;
    pVectorsTable[dwRetVector].nVectorSize = 0;
    switch (eType) {
        case EInt:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * sizeof(int));
            pVectorsTable[dwRetVector].nElemSize = sizeof(int);
            break;
        case ELong:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * sizeof(long));
            pVectorsTable[dwRetVector].nElemSize = sizeof(long);
            break;
        case EFloat:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * sizeof(float));
            pVectorsTable[dwRetVector].nElemSize = sizeof(float);
            break;
        case EDouble:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * sizeof(double));
            pVectorsTable[dwRetVector].nElemSize = sizeof(double);
            break;
        case EString:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * sizeof(wchar_t*));
            if (pVectorsTable[dwRetVector].pElementsMemory != NULL) {
                for (size_t i = 0; i < nDefaultCapacity; i++) {
                    ((wchar_t**)pVectorsTable[dwRetVector].pElementsMemory)[i] = NULL;
                }
                pVectorsTable[dwRetVector].nElemSize = sizeof(wchar_t*);
            }
            break;
        default:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * nElementSize);
            pVectorsTable[dwRetVector].nElemSize = nElementSize;
    }
    return dwRetVector;
}

BOOL VectorAddElement(DWORD dwVector, void* pElement) {
    if (dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    if (vector->nVectorSize + 1 == vector->nCapacity) {
        void* pTempMem = realloc(vector->pElementsMemory, (vector->nCapacity + nDefaultCapacity) * vector->nElemSize);
        if (pTempMem) {
            vector->pElementsMemory = pTempMem;
            vector->nCapacity += nDefaultCapacity;
        }
        else {
            return FALSE;
        }
    }
    switch (vector->eVectorType) {
        case EInt:
            ((int*)vector->pElementsMemory)[vector->nVectorSize++] = *(int*)pElement;
            break;
        case ELong:
            ((long*)vector->pElementsMemory)[vector->nVectorSize++] = *(long*)pElement;
            break;
        case EFloat:
            ((float*)vector->pElementsMemory)[vector->nVectorSize++] = *(float*)pElement;
            break;
        case EDouble:
            ((double*)vector->pElementsMemory)[vector->nVectorSize++] = *(double*)pElement;
            break;
        case EString:
            if((wchar_t**)vector->pElementsMemory != NULL) {
                wchar_t* szLine = ((wchar_t**)vector->pElementsMemory)[vector->nVectorSize];
                szLine = malloc((wcslen(pElement) + 1) * sizeof(wchar_t));
                if (szLine != NULL) {
                    wcscpy_s(szLine, wcslen(pElement) + 1, pElement);
                    ((wchar_t**)vector->pElementsMemory)[vector->nVectorSize] = szLine;
                }
                vector->nVectorSize++;
            }
            wchar_t* szLine = ((wchar_t**)vector->pElementsMemory)[vector->nVectorSize - 1];
            break;
        default:
            memcpy_s((char*)vector->pElementsMemory + vector->nVectorSize * vector->nElemSize, vector->nCapacity * vector->nElemSize, pElement, vector->nElemSize);
    }
    return TRUE;
}

void* VectorGetAt(DWORD dwVector, size_t nPosition) {
    if (dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    if (nPosition > vector->nVectorSize) {
        return FALSE;
    }
    switch (vector->eVectorType) {
        case EInt:
            return (void*)&((int*)vector->pElementsMemory)[nPosition];
        case ELong:
            return (void*)&((long*)vector->pElementsMemory)[nPosition];
        case EFloat:
            return (void*)&((float*)vector->pElementsMemory)[nPosition];
        case EDouble:
            return (void*)&((double*)vector->pElementsMemory)[nPosition];
        case EString:
            return (void*)(((wchar_t**)vector->pElementsMemory)[nPosition]);
        default:
            return (void*)((char*)vector->pElementsMemory + nPosition * vector->nElemSize);
    }
}

BOOL VectorRemoveAt(DWORD dwVector, size_t nPosition) {
    if (dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    if (nPosition > vector->nVectorSize) {
        return FALSE;
    }
    if (vector->eVectorType == EString) {
        wchar_t* pString = ((wchar_t**)vector->pElementsMemory)[nPosition];
        if (pString != NULL) {
            free(pString);
        }
    }
    if (vector->eVectorType == EStruct) {
        void* pElem = (void*)((char*)vector->pElementsMemory + nPosition + vector->nElemSize);
        if (pDestructors[dwVector] != NULL) {
            pDestructors[dwVector](pElem);
        }
    }
    // Windows don't want to deal with raw void*, pathetic
    memmove_s((char*)vector->pElementsMemory + nPosition * vector->nElemSize,
              vector->nCapacity * vector->nElemSize,
              (char*)vector->pElementsMemory + (nPosition + 1) * vector->nElemSize,
              (vector->nVectorSize - nPosition - 1) * vector->nElemSize);
    vector->nVectorSize--;
    return TRUE;
}

BOOL VectorDelete(DWORD dwVector) {
    if (dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    if (vector->eVectorType == EString) {
        if (vector->pElementsMemory != NULL) {
            for (int i = 0; i < vector->nVectorSize; i++) {
                wchar_t* szLine = ((wchar_t**)vector->pElementsMemory)[i];
                if (szLine != NULL) {
                    free(((wchar_t**)vector->pElementsMemory)[i]);
                }
            }
        }
    }
    else if (vector->eVectorType == EStruct) {
        for (int i = 0; i < vector->nVectorSize; i++) {
            void* pELement = (void*)((char*)vector->pElementsMemory + i * vector->nElemSize);
            if (pDestructors[dwVector] != NULL) {
                pDestructors[dwVector](pELement);
            }
        }
    }
    free(vector->pElementsMemory);
    if (!bClear) {
        PutUnusedVector(dwVector);
    }
    return TRUE;
}

BOOL VectorSet(DWORD dwVector, size_t nPosition, void* pValue) {
    if (dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    switch (vector->eVectorType) {
        case EInt:
            ((int*)vector->pElementsMemory)[nPosition] = *(int*)pValue;
            break;
        case ELong:
            ((long*)vector->pElementsMemory)[nPosition] = *(long*)pValue;
            break;
        case EFloat:
            ((float*)vector->pElementsMemory)[nPosition] = *(float*)pValue;
            break;
        case EDouble:
            ((double*)vector->pElementsMemory)[nPosition] = *(double*)pValue;
            break;
        case EString:
            wchar_t* szLine = ((wchar_t**)vector->pElementsMemory)[nPosition];
            if (szLine != NULL) {
                free(szLine);
            }
            szLine = malloc((wcslen(pValue) + 1) * sizeof(wchar_t));
            if (szLine != NULL) {
                wcscpy_s(szLine, (wcslen(pValue) + 1), pValue);
            }
        default:
            void* pElement = (void*)((char*)vector->pElementsMemory + nPosition * vector->nElemSize);
            if(pDestructors[dwVector] != NULL) {
                pDestructors[dwVector](pElement);
            }
            memset((char*)vector->pElementsMemory + nPosition * vector->nElemSize, 0, vector->nElemSize);
            memcpy_s((char*)vector->pElementsMemory + nPosition * vector->nElemSize, vector->nElemSize, pValue, vector->nElemSize);
    }
    return TRUE;
}

size_t VectorSize(DWORD dwVector) {
    if (dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    return pVectorsTable[dwVector].nVectorSize;
}

BOOL VectorInsertAt(DWORD dwVector, size_t nPosition, void* pValue) {
    if (dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    if (nPosition >= vector->nVectorSize) {
        return FALSE;
    }
    if (vector->nVectorSize == vector->nCapacity) {
        void* tempMem = realloc(vector->pElementsMemory, (vector->nCapacity + nDefaultCapacity) * vector->nElemSize);
        if (tempMem) {
            vector->pElementsMemory = tempMem;
        }
    }
    memmove_s((char*)vector->pElementsMemory + (nPosition + 1) * vector->nElemSize,
              vector->nCapacity * vector->nElemSize,
              (char*)vector->pElementsMemory + nPosition * vector->nElemSize,
              (vector->nVectorSize - nPosition) * vector->nElemSize);
    vector->nVectorSize++;
    return VectorSet(dwVector, nPosition, pValue);
}

EType VectorGetType(DWORD dwVector, size_t nPosition) {
    if (dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector vector = pVectorsTable[dwVector];
    return vector.eVectorType;
}

BOOL VectorSetDestroyFunc(DWORD dwList, Destructor destructor) {
    if (dwList > dwCurrentVectorsSize) {
        return FALSE;
    }
    pDestructors[dwList] = destructor;
    return TRUE;
}

void FreeVectors() {
    bClear = TRUE;
    for (DWORD i = 0; i < dwCurrentVectorsSize; i++) {
        VectorDelete(i);
    }
    free(pVectorsTable);
}