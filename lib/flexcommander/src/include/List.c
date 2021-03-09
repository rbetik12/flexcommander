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
