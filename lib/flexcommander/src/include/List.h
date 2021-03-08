#pragma once

typedef struct PathListNode {
    struct PathListNode* next;
    const char* token;
} PathListNode;
