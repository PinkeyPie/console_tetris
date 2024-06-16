#include "memory.h"
#include "string.h"
#include "Vector.h"
#include "stdlib.h"
#include "CollectionUtils.h"
#include "Collection.h"
#include "wchar.h"
#include <malloc.h>

typedef struct Vector {
    ECollection collectionType;
    void* pElementsMemory;
    EType eVectorType;
    size_t nElemSize;
    size_t nVectorSize;
    size_t nCapacity;
    Destructor pDestructor;
} Vector;

static size_t nDefaultCapacity = 10;

HANDLE VectorCreate(EType eType, size_t nElementSize) {
    Vector* newVector = malloc(sizeof(Vector));
    newVector->collectionType = EVector;
    newVector->nCapacity = nDefaultCapacity;
    newVector->eVectorType = eType;
    newVector->nVectorSize = 0;
    newVector->pDestructor = NULL;
    switch (eType) {
        case EInt:
            newVector->pElementsMemory = malloc(nDefaultCapacity * sizeof(int));
            newVector->nElemSize = sizeof(int);
            break;
        case ELong:
            newVector->pElementsMemory = malloc(nDefaultCapacity * sizeof(long));
            newVector->nElemSize = sizeof(long);
            break;
        case EFloat:
            newVector->pElementsMemory = malloc(nDefaultCapacity * sizeof(float));
            newVector->nElemSize = sizeof(float);
            break;
        case EDouble:
            newVector->pElementsMemory = malloc(nDefaultCapacity * sizeof(double));
            newVector->nElemSize = sizeof(double);
            break;
        case EString:
            newVector->pElementsMemory = malloc(nDefaultCapacity * sizeof(wchar_t*));
            if (newVector->pElementsMemory != NULL) {
                for (size_t i = 0; i < nDefaultCapacity; i++) {
                    ((wchar_t**)newVector->pElementsMemory)[i] = NULL;
                }
                newVector->nElemSize = sizeof(wchar_t*);
            }
            break;
        default:
            newVector->pElementsMemory = malloc(nDefaultCapacity * nElementSize);
            newVector->nElemSize = nElementSize;
    }
    return newVector;
}

BOOL VectorAddElement(HANDLE hVector, void* pElement) {
    Vector* vector = (Vector*)hVector;
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
#ifdef _MSC_VER
                    wcscpy_s(szLine, wcslen(pElement) + 1, pElement);
#else
                    wcscpy(szLine, pElement);
#endif
                    ((wchar_t**)vector->pElementsMemory)[vector->nVectorSize] = szLine;
                }
                vector->nVectorSize++;
            }
            break;
        default:
#ifdef _MSC_VER
            memcpy_s((char*)vector->pElementsMemory + vector->nVectorSize * vector->nElemSize,
                     vector->nCapacity * vector->nElemSize,
                     pElement,
                     vector->nElemSize);
#else
            memcpy((char*)vector->pElementsMemory + vector->nVectorSize * vector->nElemSize,
               pElement,
               vector->nElemSize);
#endif
            vector->nVectorSize++;
    }
    return TRUE;
}

void* VectorGetAt(HANDLE hVector, size_t nPosition) {
    Vector* vector = (Vector*)hVector;
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
        case EString: {
            wchar_t *szLine = (wchar_t *) (((wchar_t **) vector->pElementsMemory)[nPosition]);
            return szLine;
        }
        default:
            return (void*)((char*)vector->pElementsMemory + nPosition * vector->nElemSize);
    }
}

BOOL VectorRemoveAt(HANDLE hVector, size_t nPosition) {
    Vector* vector = (Vector*)hVector;
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
        if(vector->pDestructor != NULL) {
            vector->pDestructor(pElem);
        }
    }
    // Windows don't want to deal with raw void*, pathetic
#ifdef _MSC_VER
    memmove_s((char*)vector->pElementsMemory + nPosition * vector->nElemSize,
              vector->nCapacity * vector->nElemSize,
              (char*)vector->pElementsMemory + (nPosition + 1) * vector->nElemSize,
              (vector->nVectorSize - nPosition - 1) * vector->nElemSize);
#else
    memmove((char*)vector->pElementsMemory + nPosition * vector->nElemSize,
              (char*)vector->pElementsMemory + (nPosition + 1) * vector->nElemSize,
              (vector->nVectorSize - nPosition - 1) * vector->nElemSize);
#endif
    vector->nVectorSize--;
    return TRUE;
}

BOOL VectorDelete(HANDLE hVector) {
    Vector* vector = (Vector*)hVector;
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
            void* pElement = (void*)((char*)vector->pElementsMemory + i * vector->nElemSize);
            if(vector->pDestructor != NULL) {
                vector->pDestructor(pElement);
            }
        }
    }
    free(vector->pElementsMemory);
    free(vector);
    return TRUE;
}

BOOL VectorSet(HANDLE hVector, size_t nPosition, void* pValue) {
    Vector* vector = (Vector*)hVector;
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
        case EString: {
            wchar_t *szLine = ((wchar_t **) vector->pElementsMemory)[nPosition];
            if (szLine != NULL) {
                free(szLine);
            }
            szLine = malloc((wcslen(pValue) + 1) * sizeof(wchar_t));
            if (szLine != NULL) {
#ifdef _MSC_VER
                wcscpy_s(szLine, (wcslen(pValue) + 1), pValue);
#else
                wcscpy(szLine, pValue);
#endif
            }
            ((wchar_t **) vector->pElementsMemory)[nPosition] = szLine;
            break;
        }
        default: {
            void *pElement = (void *) ((char *) vector->pElementsMemory + nPosition * vector->nElemSize);
            if (vector->pDestructor != NULL) {
                vector->pDestructor(pElement);
            }
            memset((char *) vector->pElementsMemory + nPosition * vector->nElemSize, 0, vector->nElemSize);
#ifdef _MSC_VER
            memcpy_s((char*)vector->pElementsMemory + nPosition * vector->nElemSize,
                     vector->nElemSize,
                     pValue,
                     vector->nElemSize);
#else
            memcpy((char *) vector->pElementsMemory + nPosition * vector->nElemSize,
               pValue,
               vector->nElemSize);
#endif
        }
    }
    return TRUE;
}

size_t VectorSize(HANDLE hVector) {
    Vector* vector = (Vector*)hVector;
    return vector->nVectorSize;
}

BOOL VectorInsertAt(HANDLE hVector, size_t nPosition, void* pValue) {
    Vector* vector = (Vector*)hVector;
    if (nPosition >= vector->nVectorSize) {
        return FALSE;
    }
    if (vector->nVectorSize == vector->nCapacity) {
        void* tempMem = realloc(vector->pElementsMemory, (vector->nCapacity + nDefaultCapacity) * vector->nElemSize);
        if (tempMem) {
            vector->pElementsMemory = tempMem;
        }
    }
#ifdef _MSC_VER
    memmove_s((char*)vector->pElementsMemory + (nPosition + 1) * vector->nElemSize,
              vector->nCapacity * vector->nElemSize,
              (char*)vector->pElementsMemory + nPosition * vector->nElemSize,
              (vector->nVectorSize - nPosition) * vector->nElemSize);
#else
    memmove((char*)vector->pElementsMemory + (nPosition + 1) * vector->nElemSize,
            (char*)vector->pElementsMemory + nPosition * vector->nElemSize,
            (vector->nVectorSize - nPosition) * vector->nElemSize);
#endif
    vector->nVectorSize++;
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
        case EString: {
            wchar_t *szLine = malloc((wcslen(pValue) + 1) * sizeof(wchar_t));
            if (szLine != NULL) {
#ifdef _MSC_VER
                wcscpy_s(szLine, (wcslen(pValue) + 1), pValue);
#else
                wcscpy(szLine, pValue);
#endif
            }
            ((wchar_t **) vector->pElementsMemory)[nPosition] = szLine;
            break;
        }
        default: {
            memset((char *) vector->pElementsMemory + nPosition * vector->nElemSize, 0, vector->nElemSize);
#ifdef _MSC_VER
            memcpy_s((char*)vector->pElementsMemory + nPosition * vector->nElemSize,
                     vector->nElemSize,
                     pValue,
                     vector->nElemSize);
#else
            memcpy((char *) vector->pElementsMemory + nPosition * vector->nElemSize,
                   pValue,
                   vector->nElemSize);
#endif
        }
    }
    return TRUE;
}

EType VectorGetType(HANDLE hVector, size_t nPosition) {
    Vector* vector = (Vector*)hVector;
    return vector->eVectorType;
}

BOOL VectorSetDestroyFunc(HANDLE hVector, Destructor destructor) {
    if(hVector != NULL) {
        Vector* vector = (Vector*)hVector;
        vector->pDestructor = destructor;
    }
    return TRUE;
}