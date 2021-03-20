#pragma once

#include <stdint.h>
#include <Flexcommander.h>
#include <HFSPlusBTree.h>

typedef struct BTHeaderRec BTHeaderRec;

void MakePath(char* dir);
void CopyFile(const char* dest, const char* filename, HFSPlusCatalogFile file, FlexCommanderFS *fs);
void CopyDirectory(const char* _src, const char* _dest, uint32_t parentID, BTHeaderRec btreeHeader, FlexCommanderFS fs);

typedef struct CopyInfo {
    char* dest;
} CopyInfo;