#pragma once

#include <HFSCatalog.h>

typedef struct PathListNode {
    struct PathListNode* next;
    const char* token;
} PathListNode;

typedef struct FSRecordListNode {
    uint32_t cnid;
    HFSUniStr255 name;
    enum HFSDataRecordType type;
    struct FSRecordListNode* next;
} FSRecordListNode;

void FSRecordListAdd(FSRecordListNode** listHead, FSRecordListNode newNode);
