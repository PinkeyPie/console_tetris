#include "CollectionUtils.h"
#include "stdlib.h"

typedef struct UnusedNode {
    DWORD dwUnused;
    struct UnusedNode* pNext;
} UnusedNode;

static struct UnusedNode* pUnusedLists;
static struct UnusedNode* pUnusedVectors;

BOOL GetUnused(UnusedNode* pUnusedCollections, DWORD* pDwUnused) {
    if (pUnusedCollections == NULL) {
        return FALSE;
    }
    else {
        *pDwUnused = pUnusedCollections->dwUnused;
        UnusedNode* pTempMem = pUnusedCollections;
        pUnusedCollections = pUnusedCollections->pNext;
        free(pTempMem);
        return TRUE;
    }
}

void PutUnused(UnusedNode* pUnusedCollections, DWORD dwUnused) {
    if (pUnusedCollections == NULL) {
        pUnusedCollections = malloc(sizeof(UnusedNode));
        pUnusedCollections->dwUnused = dwUnused;
    }
    else {
        UnusedNode* pCurrent = pUnusedCollections;
        while (pCurrent->pNext != NULL) {
            pCurrent = pCurrent->pNext;
        }
        UnusedNode* pNewNode = malloc(sizeof(UnusedNode));
        pNewNode->dwUnused = dwUnused;
        pCurrent->pNext = pNewNode;
    }
}

void FreeUnusedStructs() {
    while (pUnusedLists != NULL) {
        UnusedNode* pDelMem = pUnusedLists;
        pUnusedLists = pUnusedLists->pNext;
        free(pDelMem);
    }
    while (pUnusedVectors != NULL) {
        UnusedNode* pDelMem = pUnusedVectors;
        pUnusedVectors = pUnusedVectors->pNext;
        free(pDelMem);
    }
}

BOOL GetUnusedVector(DWORD* pDwUnused) {
    return GetUnused(pUnusedVectors, pDwUnused);
}

BOOL GetUnusedList(DWORD* pDwUnused) {
    return GetUnused(pUnusedLists, pDwUnused);
}

void PutUnusedVector(DWORD pDwUnused) {
    PutUnused(pUnusedVectors, pDwUnused);
}

void PutUnusedList(DWORD pDwUnused) {
    PutUnused(pUnusedLists, pDwUnused);
}