#include <wchar.h>
#include "List.h"
#include "Collection.h"
#include "CollectionUtils.h"
#include "stdlib.h"

typedef struct _ListNode {
    void* pValue;
    EType valueType;
    struct _ListNode* pNext;
    Destructor destructor;
} ListNode;

typedef struct _List {
    ECollection collectionType;
    ListNode* root;
    size_t size;
    Destructor destructor;
    BOOL oneType;
} List;

HANDLE ListCreate() {
    DWORD dwNewList = 0;
    List* list = malloc(sizeof(List));
    if(list == NULL) {
        return NULL;
    }
    list->root = NULL;
    list->size = 0;
    list->destructor = NULL;
    list->oneType = FALSE;
    list->collectionType = ELinkedList;
    return list;
}

BOOL ListAddElement(HANDLE hList, void* pElement, EType eType) {
    List* list = (List*)hList;
    if(pElement == NULL || list->oneType && list->root->valueType != eType) {
        return FALSE;
    }
    ListNode* pRoot = list->root;
    if (list->size == 0) {
        ListNode* node = malloc(sizeof(ListNode));
        list->root = node;
        pRoot = list->root;
        if (pElement != NULL) {
            pRoot->pValue = pElement;
            pRoot->valueType = eType;
            pRoot->pNext = NULL;
            pRoot->destructor = NULL;
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
            pNewItem->destructor = NULL;
            pCurrItem->pNext = pNewItem;
        }
    }
    list->size++;
    return TRUE;
}

BOOL ListAddStruct(HANDLE hList, void* pElem, Destructor destructor) {
    List* list = (List*)hList;
    if(list == NULL || destructor == NULL || pElem == NULL) {
        return FALSE;
    }
    BOOL result = ListAddElement(hList, pElem, EStruct);
    if(!result) {
        return FALSE;
    }
    ListNode* node = list->root;
    while (node->pNext != NULL) {
        node = node->pNext;
    }
    node->destructor = destructor;
    return TRUE;
}

void* ListGetAt(HANDLE hList, size_t nPosition, EType* eType) {
    List* list = (List*)hList;
    if(list->size < nPosition) {
        return NULL;
    }
    int nIter = 0;
    ListNode* pCurrent = list->root;
    while (nIter < nPosition) {
        pCurrent = pCurrent->pNext;
        nIter++;
    }
    *eType = pCurrent->valueType;
    return pCurrent->pValue;
}

BOOL ListRemoveAt(HANDLE hList, size_t nPosition) {
    List* list = (List*)hList;
    if(list->size < nPosition) {
        return FALSE;
    }

    if (nPosition == 0) {
        ListNode* pDelMem = list->root;
        if (pDelMem->pValue != NULL) {
            if (list->oneType && list->destructor != NULL) {
                list->destructor(pDelMem->pValue);
            } else if(pDelMem->destructor != NULL) {
                pDelMem->destructor(pDelMem->pValue);
            }
            free(pDelMem->pValue);
            pDelMem->pValue = NULL;
        }
        if (pDelMem->pNext != NULL) {
            list->root = list->root->pNext;
            free(pDelMem);
        }
    }
    else {
        int nIter = 0;
        ListNode* pCurrent = list->root;
        while (nIter != nPosition - 1) {
            pCurrent = pCurrent->pNext;
            nIter++;
        }
        if (pCurrent->pNext->pValue != NULL) {
            if (list->oneType && list->destructor != NULL) {
                list->destructor(pCurrent->pNext->pValue);
            } else if(pCurrent->pNext->destructor != NULL) {
                pCurrent->pNext->destructor(pCurrent->pNext->pValue);
            }
            free(pCurrent->pNext->pValue);
            pCurrent->pNext->pValue = NULL;
        }
        ListNode* delMem = pCurrent->pNext;
        pCurrent->pNext = pCurrent->pNext->pNext;
        free(delMem);
    }
    list->size--;
    return TRUE;
}

BOOL ListDelete(HANDLE hList) {
    List* list = (List*)hList;
    ListNode* pCurrent = list->root;
    while (pCurrent != NULL) {
        if (pCurrent->pValue != NULL) {
            if (list->oneType && list->destructor != NULL) {
                list->destructor(pCurrent->pValue);
            } else if(pCurrent->destructor != NULL) {
                pCurrent->destructor(pCurrent->pValue);
            }
            free(pCurrent->pValue);
            pCurrent->pValue = NULL;
        }
        ListNode* pDelMem = pCurrent;
        pCurrent = pCurrent->pNext;
        pDelMem->pNext = NULL;
        free(pDelMem);
    }
    free(list);
    return TRUE;
}

size_t ListSize(HANDLE hList) {
    List* list = (List*)hList;
    return list->size;
}

BOOL ListSet(HANDLE hList, size_t nPosition, void* pValue, EType eType) {
    List* list = (List*)hList;
    if (nPosition >= list->size) {
        return FALSE;
    }
    int nIter = 0;
    ListNode* pCurrent = list->root;
    while (nIter < nPosition) {
        pCurrent = pCurrent->pNext;
        nIter++;
    }
    if (pCurrent->pValue != NULL) {
        if (list->oneType && list->destructor != NULL) {
            list->destructor(pCurrent->pValue);
        } else if(pCurrent->destructor != NULL) {
            pCurrent->destructor(pCurrent->pValue);
        }
        free(pCurrent->pValue);
        pCurrent->pValue = NULL;
    }
    pCurrent->pValue = pValue;
    pCurrent->valueType = eType;
    return TRUE;
}

BOOL ListSetStruct(HANDLE hList, size_t position, void* pValue, Destructor destructor) {
    List* list = (List*)hList;
    if (position >= list->size || destructor == NULL) {
        return FALSE;
    }
    int nIter = 0;
    ListNode* pCurrent = list->root;
    while (nIter < position) {
        pCurrent = pCurrent->pNext;
        nIter++;
    }
    if (pCurrent->pValue != NULL) {
        if (list->oneType && list->destructor != NULL) {
            list->destructor(pCurrent->pValue);
        } else if(pCurrent->destructor != NULL) {
            pCurrent->destructor(pCurrent->pValue);
        }
        free(pCurrent->pValue);
        pCurrent->pValue = NULL;
    }
    pCurrent->pValue = pValue;
    pCurrent->valueType = EStruct;
    pCurrent->destructor = destructor;
    return TRUE;
}

BOOL ListInsertAt(HANDLE hList, size_t nPosition, void* pValue, EType eType) {
    List* list = (List*)hList;
    if (nPosition >= list->size) {
        return FALSE;
    }
    int nIter = 0;
    ListNode* pCurrent = list->root;
    while (nIter < nPosition) {
        pCurrent = pCurrent->pNext;
        nIter++;
    }
    ListNode* pNewNode = malloc(sizeof(ListNode));
    pNewNode->pValue = pCurrent->pValue;
    pNewNode->valueType = pCurrent->valueType;
    pNewNode->pNext = pCurrent->pNext;
    pNewNode->destructor = NULL;
    pCurrent->pValue = pValue;
    pCurrent->valueType = eType;
    pCurrent->pNext = pNewNode;
    list->size++;
    return TRUE;
}

BOOL ListInsertAtStruct(HANDLE hList, size_t nPosition, void* pValue, Destructor destructor) {
    List* list = (List*)hList;
    if (nPosition >= list->size) {
        return FALSE;
    }
    int nIter = 0;
    ListNode* pCurrent = list->root;
    while (nIter < nPosition) {
        pCurrent = pCurrent->pNext;
        nIter++;
    }
    ListNode* pNewNode = malloc(sizeof(ListNode));
    pNewNode->pValue = pCurrent->pValue;
    pNewNode->valueType = pCurrent->valueType;
    pNewNode->pNext = pCurrent->pNext;
    pNewNode->destructor = NULL;
    pCurrent->pValue = pValue;
    pCurrent->valueType = EStruct;
    pCurrent->destructor = destructor;
    pCurrent->pNext = pNewNode;
    list->size++;
    return TRUE;
}

EType ListGetType(HANDLE hList, size_t nPosition) {
    List* list = (List*)hList;
    if (nPosition > list->size) {
        return EUnknown;
    }
    ListNode* pCurrent = list->root;
    int nIter = 0;
    while (nIter < nPosition) {
        pCurrent = pCurrent->pNext;
        nIter++;
    }
    return pCurrent->valueType;
}

BOOL ListSetDestroyFunc(HANDLE hList, Destructor destructor) {
    List* list = (List*)list;
    if(destructor == NULL) {
        return FALSE;
    }
    list->destructor = destructor;
    list->oneType = TRUE;
    return TRUE;
}