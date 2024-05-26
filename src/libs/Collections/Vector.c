#include <mem.h>
#include "Vector.h"
#include "stdlib.h"
#include "CollectionUtils.h"

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

DWORD VectorCreate(EType eType, size_t nElementSize) {
    DWORD dwRetVector = 0;
    if(pVectorsTable == NULL) {
        pVectorsTable = malloc(sizeof(Vector) * dwNewItemsCount);
        dwCurrentTableSize += dwNewItemsCount;
    } else if(dwCurrentVectorsSize >= dwCurrentTableSize) {
        Vector* pTempMem = realloc(pVectorsTable, dwCurrentTableSize + dwNewItemsCount);
        if(pTempMem) {
            pVectorsTable = pTempMem;
        }
        dwCurrentTableSize += dwNewItemsCount;
    }
    dwRetVector = dwCurrentVectorsSize++;
    pVectorsTable[dwRetVector].nCapacity = nDefaultCapacity;
    pVectorsTable[dwRetVector].eVectorType = eType;
    pVectorsTable[dwRetVector].nVectorSize = 0;
    switch (eType) {
        case Int:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * sizeof(int));
            pVectorsTable[dwRetVector].nElemSize = sizeof(int);
            break;
        case Long:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * sizeof(long));
            pVectorsTable[dwRetVector].nElemSize = sizeof(long);
            break;
        case Float:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * sizeof(float));
            pVectorsTable[dwRetVector].nElemSize = sizeof(float);
            break;
        case Double:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * sizeof(double));
            pVectorsTable[dwRetVector].nElemSize = sizeof(double );
            break;
        case String:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * sizeof(wchar_t*));
            pVectorsTable[dwRetVector].nElemSize = sizeof(wchar_t*);
            break;
        default:
            pVectorsTable[dwRetVector].pElementsMemory = malloc(nDefaultCapacity * nElementSize);
            pVectorsTable[dwRetVector].nElemSize = nElementSize;
    }
    return dwRetVector;
}

BOOL VectorAddElement(DWORD dwVector, void* pElement) {
    if(dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    if(vector->nVectorSize + 1 == vector->nCapacity) {
        void* pTempMem = realloc(vector->pElementsMemory, (vector->nCapacity + nDefaultCapacity) * vector->nElemSize);
        if(pTempMem) {
            vector->pElementsMemory = pTempMem;
            vector->nCapacity += nDefaultCapacity;
        }
    }
    switch (vector->eVectorType) {
        case Int:
            ((int*)vector->pElementsMemory)[vector->nVectorSize++] = *(int*)pElement;
            break;
        case Long:
            ((long*)vector->pElementsMemory)[vector->nVectorSize++] = *(long*)pElement;
            break;
        case Float:
            ((float*)vector->pElementsMemory)[vector->nVectorSize++] = *(float*)pElement;
            break;
        case Double:
            ((double*)vector->pElementsMemory)[vector->nVectorSize++] = *(double*)pElement;
            break;
        case String:
            ((wchar_t**)vector->pElementsMemory)[vector->nVectorSize++] = malloc((wcslen(pElement) + 1) * sizeof(wchar_t));
            wcscpy_s(((wchar_t**)vector->pElementsMemory)[vector->nVectorSize++], (wcslen(pElement) + 1), pElement);
        default:
            ((void**)vector->pElementsMemory)[vector->nVectorSize] = malloc(vector->nElemSize);
            memcpy(((void**)vector->pElementsMemory)[vector->nVectorSize++], pElement, vector->nElemSize);
    }
    return TRUE;
}

void* VectorGetAt(DWORD dwVector, size_t nPosition) {
    if(dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    if(nPosition > vector->nVectorSize) {
        return FALSE;
    }
    switch (vector->eVectorType) {
        case Int:
            return (void*)&((int*)vector->pElementsMemory)[nPosition];
        case Long:
            return (void*)&((long*)vector->pElementsMemory)[nPosition];
        case Float:
            return (void*)&((float*)vector->pElementsMemory)[nPosition];
        case Double:
            return (void*)&((double*)vector->pElementsMemory)[nPosition];
        case String:
            return (void*)&((wchar_t**)vector->pElementsMemory)[nPosition];
        default:
            return ((void**)vector->pElementsMemory + nPosition * vector->nElemSize);
    }
}

BOOL VectorRemoveAt(DWORD dwVector, size_t nPosition) {
    if(dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    if(nPosition > vector->nVectorSize) {
        return FALSE;
    }
    memmove(vector->pElementsMemory + nPosition * vector->nElemSize,
            vector->pElementsMemory + (nPosition + 1) * vector->nElemSize,
            (vector->nVectorSize - nPosition - 1) * vector->nElemSize);
    vector->nVectorSize--;
    return TRUE;
}

BOOL VectorDelete(DWORD dwVector) {
    if(dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    if(vector->eVectorType == String) {
        for(int i = 0; i < vector->nVectorSize; i++) {
            free(((wchar_t**)vector->pElementsMemory)[i]);
        }
    } else if(vector->eVectorType == Struct) {
        for(int i = 0; i < vector->nVectorSize; i++) {
            free(((wchar_t**)vector->pElementsMemory)[i]);
        }
    }
    free(vector->pElementsMemory);
    if(!bClear) {
        PutUnusedVector(dwVector);
    }
    return TRUE;
}

BOOL VectorSet(DWORD dwVector, size_t nPosition, void* pValue) {
    if(dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    switch (vector->eVectorType) {
        case Int:
            ((int*)vector->pElementsMemory)[nPosition] = *(int*)pValue;
            break;
        case Long:
            ((long*)vector->pElementsMemory)[nPosition] = *(long*)pValue;
            break;
        case Float:
            ((float*)vector->pElementsMemory)[nPosition] = *(float*)pValue;
            break;
        case Double:
            ((double*)vector->pElementsMemory)[nPosition] = *(double*)pValue;
            break;
        case String:
            free(((wchar_t**)vector->pElementsMemory)[nPosition]);
            ((wchar_t**)vector->pElementsMemory)[nPosition] = malloc((wcslen(pValue) + 1) * sizeof(wchar_t));
            wcscpy_s(((wchar_t**)vector->pElementsMemory)[nPosition], (wcslen(pValue) + 1), pValue);
        default:
            free(((void**)vector->pElementsMemory)[nPosition]);
            ((void**)vector->pElementsMemory)[nPosition] = malloc(vector->nElemSize);
            memcpy_s(((void**)vector->pElementsMemory)[nPosition], vector->nElemSize, pValue, vector->nElemSize);
    }
    return TRUE;
}

size_t VectorSize(DWORD dwVector) {
    if(dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    return pVectorsTable[dwVector].nVectorSize;
}

BOOL VectorInsertAt(DWORD dwVector, size_t nPosition, void* pValue) {
    if(dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector* vector = &pVectorsTable[dwVector];
    if(nPosition >= vector->nVectorSize) {
        return FALSE;
    }
    if(vector->nVectorSize == vector->nCapacity) {
        void* tempMem = realloc(vector->pElementsMemory, (vector->nCapacity + nDefaultCapacity) * vector->nElemSize);
        if(tempMem) {
            vector->pElementsMemory = tempMem;
        }
    }
    memmove(vector->pElementsMemory + (nPosition + 1) * vector->nElemSize,
            vector->pElementsMemory + nPosition * vector->nElemSize,
            (vector->nVectorSize - nPosition) * vector->nElemSize);
    vector->nVectorSize++;
    return VectorSet(dwVector, nPosition, pValue);
}

EType VectorGetType(DWORD dwVector, size_t nPosition) {
    if(dwVector > dwCurrentVectorsSize) {
        return FALSE;
    }
    Vector vector = pVectorsTable[dwVector];
    return vector.eVectorType;
}

void FreeVectors() {
    bClear = TRUE;
    for(DWORD i = 0; i < dwCurrentVectorsSize; i++) {
        VectorDelete(i);
    }
    free(pVectorsTable);
}