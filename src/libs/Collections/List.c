#include <wchar.h>
#include "List.h"
#include "Collection.h"
#include "CollectionUtils.h"
#include "stdlib.h"

typedef struct ListNode {
    void* pValue;
    EType valueType;
    struct ListNode* pNext;
} ListNode;

static DWORD dwCurrentSize = 0;
static struct ListNode* pListsTable = NULL;
static size_t* pListsSizes = NULL;
static Destructor* pDestructors = NULL;
static DWORD dwCurrentTableSize = 0;
static unsigned const nCountNewItems = 10;
static BOOL bFinalize = FALSE;

DWORD ListCreate() {
    DWORD dwNewList = 0;
    if (pListsTable == NULL) {
        pListsTable = malloc(sizeof(ListNode) * nCountNewItems);
        pListsSizes = malloc(sizeof(size_t) * nCountNewItems);
        pDestructors = malloc(sizeof(Destructor) * nCountNewItems);
        for (DWORD i = 0; i < nCountNewItems; i++) {
            pListsTable[i].pNext = NULL;
            pListsTable[i].pValue = NULL;
            pListsTable[i].valueType = EUnknown;
            pListsSizes[i] = 0;
            pDestructors[i] = NULL;
        }
        dwCurrentTableSize += nCountNewItems;
    }
    if (dwCurrentSize >= dwCurrentTableSize) {
        ListNode* pTempMem = realloc(pListsTable, (dwCurrentTableSize + nCountNewItems) * sizeof(ListNode));
        if (pTempMem) {
            pListsTable = pTempMem;
            for (DWORD i = dwCurrentTableSize; i < dwCurrentTableSize + nCountNewItems; i++) {
                pListsTable[i].pNext = NULL;
                pListsTable[i].pValue = NULL;
                pListsTable[i].valueType = EUnknown;
            }
        }
        size_t* pTempSizes = realloc(pListsSizes, (dwCurrentTableSize + nCountNewItems) * sizeof(size_t));
        if (pTempSizes) {
            pListsSizes = pTempSizes;
            for (DWORD i = dwCurrentTableSize; i < dwCurrentTableSize + nCountNewItems; i++) {
                pListsSizes[i] = 0;
            }
        }
        Destructor* pTempDestructors = realloc(pDestructors, (dwCurrentTableSize + nCountNewItems) * sizeof(Destructor));
        if (pTempDestructors) {
            pDestructors = pTempDestructors;
            for (DWORD i = dwCurrentTableSize; i < dwCurrentTableSize + nCountNewItems; i++) {
                pDestructors[i] = NULL;
            }
        }
        if (pTempSizes && pTempMem) {
            dwCurrentTableSize += nCountNewItems;
        }
    }
    dwNewList = dwCurrentSize++;
    return dwNewList;
}

BOOL ListAddElement(DWORD dwList, void* pElement, EType eType) {
    if (dwList > dwCurrentSize) {
        return FALSE;
    }
    ListNode* pRoot = &pListsTable[dwList];
    if (pListsSizes[dwList] == 0) {
        if (pElement != NULL) {
            pRoot->pValue = pElement;
            pRoot->valueType = eType;
            pRoot->pNext = NULL;
        }
        else {
            return FALSE;
        }
    }
    else {
        ListNode* pCurrItem = pRoot;
        while (pCurrItem->pNext != NULL) {
            pCurrItem = pCurrItem->pNext;
        }
        if (pElement != NULL) {
            ListNode* pNewItem = malloc(sizeof(ListNode));
            pNewItem->pValue = pElement;
            pNewItem->valueType = eType;
            pNewItem->pNext = NULL;
            pCurrItem->pNext = pNewItem;
        }
    }
    pListsSizes[dwList]++;
    return TRUE;
}

void* ListGetAt(DWORD dwList, size_t nPosition, EType* pEType) {
    if (dwList > dwCurrentSize) {
        return NULL;
    }
    if (nPosition >= pListsSizes[dwList]) {
        return NULL;
    }
    int nIter = 0;
    ListNode* pCurrent = &pListsTable[dwList];
    while (nIter < nPosition) {
        pCurrent = pCurrent->pNext;
        nIter++;
    }
    return pCurrent->pValue;
}

BOOL ListRemoveAt(DWORD dwList, size_t nPosition) {
    if (dwList > dwCurrentSize) {
        return FALSE;
    }
    if (nPosition >= pListsSizes[dwList]) {
        return FALSE;
    }
    if (nPosition == 0) {
        ListNode* pDelMem = pListsTable[dwList].pNext;
        if (pListsTable[dwList].pValue != NULL) {
            if (pDestructors[dwList] != NULL) {
                pDestructors[dwList](pListsTable[dwList].pValue);
            }
            free(pListsTable[dwList].pValue);
            pListsTable[dwList].pValue = NULL;
        }
        if (pDelMem != NULL) {
            pListsTable[dwList].pValue = pDelMem->pValue;
            pListsTable[dwList].valueType = pDelMem->valueType;
            pListsTable[dwList].pNext = pDelMem->pNext;
            free(pDelMem);
        }
    }
    else {
        int nIter = 0;
        ListNode* pCurrent = &pListsTable[dwList];
        while (nIter != nPosition - 1) {
            pCurrent = pCurrent->pNext;
            nIter++;
        }
        if (pCurrent->pNext->pValue != NULL) {
            if (pDestructors[dwList] != NULL) {
                pDestructors[dwList](pCurrent->pNext->pValue);
            }
            free(pCurrent->pNext->pValue);
            pCurrent->pNext->pValue = NULL;
        }
        ListNode* delMem = pCurrent->pNext;
        pCurrent->pNext = pCurrent->pNext->pNext;
        free(delMem);
    }
    pListsSizes[dwList]--;
    return TRUE;
}

BOOL ListDelete(DWORD dwList) {
    if (dwList > dwCurrentSize) {
        return FALSE;
    }
    ListNode* pRoot = &pListsTable[dwList];
    if (pRoot->pValue != NULL) {
        if (pDestructors[dwList] != NULL) {
            pDestructors[dwList](pRoot->pValue);
        }
        free(pRoot->pValue);
        pRoot->pValue = NULL;
    }
    ListNode* pCurrent = pRoot->pNext;
    pRoot->pNext = NULL;
    while (pCurrent != NULL) {
        if (pCurrent->pValue != NULL) {
            if (pDestructors[dwList] != NULL) {
                pDestructors[dwList](pCurrent->pValue);
            }
            free(pCurrent->pValue);
            pCurrent->pValue = NULL;
        }
        ListNode* pDelMem = pCurrent;
        pCurrent = pCurrent->pNext;
        pDelMem->pNext = NULL;
        free(pDelMem);
    }
    if (!bFinalize) {
        PutUnusedList(dwList);
    }
    pListsSizes[dwList] = 0;
    return TRUE;
}

size_t ListSize(DWORD dwList) {
    if (dwCurrentTableSize < dwList) {
        return -1;
    }
    return pListsSizes[dwList];
}

BOOL ListSet(DWORD dwList, size_t nPosition, void* pValue, EType eType) {
    if (dwList > dwCurrentSize) {
        return FALSE;
    }
    if (nPosition >= pListsSizes[dwList]) {
        return FALSE;
    }
    int nIter = 0;
    ListNode* pCurrent = &pListsTable[dwList];
    while (nIter < nPosition) {
        pCurrent = pCurrent->pNext;
        nIter++;
    }
    if (pCurrent->pValue != NULL) {
        if (pDestructors[dwList] != NULL) {
            pDestructors[dwList](pCurrent->pValue);
        }
        free(pCurrent->pValue);
        pCurrent->pValue = NULL;
    }
    pCurrent->pValue = pValue;
    pCurrent->valueType = eType;
    return TRUE;
}

BOOL ListInsertAt(DWORD dwList, size_t nPosition, void* pValue, EType eType) {
    if (dwList > dwCurrentSize) {
        return FALSE;
    }
    if (nPosition >= pListsSizes[dwList]) {
        return FALSE;
    }
    int nIter = 0;
    ListNode* pCurrent = &pListsTable[dwList];
    while (nIter < nPosition) {
        pCurrent = pCurrent->pNext;
        nIter++;
    }
    ListNode* pNewNode = malloc(sizeof(ListNode));
    pNewNode->pValue = pCurrent->pValue;
    pNewNode->valueType = pCurrent->valueType;
    pNewNode->pNext = pCurrent->pNext;
    pCurrent->pValue = pValue;
    pCurrent->valueType = eType;
    pCurrent->pNext = pNewNode;
    pListsSizes[dwList]++;
    return TRUE;
}

EType ListGetType(DWORD dwList, size_t nPosition) {
    if (dwList > dwCurrentSize) {
        return EUnknown;
    }
    if (nPosition > pListsSizes[dwList]) {
        return EUnknown;
    }
    ListNode* pCurrent = &pListsTable[dwList];
    int nIter = 0;
    while (nIter < nPosition) {
        pCurrent = pCurrent->pNext;
        nIter++;
    }
    return pCurrent->valueType;
}

BOOL ListSetDestroyFunc(DWORD dwList, Destructor destructor) {
    if (dwList > dwCurrentSize) {
        return FALSE;
    }
    pDestructors[dwList] = destructor;
    return TRUE;
}

void FreeList() {
    bFinalize = TRUE;
    if (pListsTable != NULL) {
        for (DWORD i = 0; i < dwCurrentSize; i++) {
            ListDelete(i);
        }
        free(pListsTable);
        free(pListsSizes);
    }
}