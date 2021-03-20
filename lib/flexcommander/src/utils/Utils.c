#include <string.h>
#include <stdlib.h>
#include "Utils.h"

PathListNode *SplitPathWithDelimeter(char *path, const char* delimeter) {
    PathListNode *listHead = NULL;
    char *pathToken;

    while ((pathToken = strsep(&path, delimeter))) {
        if (strcmp(pathToken, "") == 0) {
            continue;
        }
        PathListNode newNode;
        memset(&newNode, 0, sizeof(PathListNode));
        newNode.token = calloc(sizeof(char), strlen(pathToken) + 1);
        newNode.token = strcpy(newNode.token, pathToken);
        PathListAdd(&listHead, newNode);
    }

    return listHead;
}
