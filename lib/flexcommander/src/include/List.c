#include <stdlib.h>
#include "List.h"

void FSRecordListAdd(FSRecordListNode** listHead, FSRecordListNode newNode) {
    FSRecordListNode* newNodePtr = malloc(sizeof(FSRecordListNode));
    FSRecordListNode* last = *listHead;
    newNodePtr->cnid = newNode.cnid;
    newNodePtr->type = newNode.type;
    newNodePtr->name = newNode.name;
    newNodePtr->next = NULL;

    if (*listHead == NULL) {
        *listHead = newNodePtr;
        return;
    }

    while(last->next != NULL) {
        last = last->next;
    }

    last->next = newNodePtr;
}

void PathListAdd(PathListNode** listHead, PathListNode newNode) {
    PathListNode * newNodePtr = malloc(sizeof(PathListNode));
    PathListNode * last = *listHead;
    newNodePtr->token = newNode.token;
    newNodePtr->_cnid = newNode._cnid;
    newNodePtr->next = NULL;

    if (*listHead == NULL) {
        *listHead = newNodePtr;
        return;
    }

    while(last->next != NULL) {
        last = last->next;
    }

    last->next = newNodePtr;
}

PathListNode* GetPathListLastNode(PathListNode** listHead) {
    PathListNode* listStart = *listHead;

    while(listStart->next != NULL) {
        listStart = listStart->next;
    }

    return listStart;
}

void PathListClear(PathListNode *listHead) {
    PathListNode *start = listHead;
    PathListNode *prev = NULL;

    while (start) {
        prev = start;
        start = start->next;

        if (prev) {
            free(prev);
        }
    }
}
