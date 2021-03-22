#pragma once

#include <stdint.h>
#include <HFSPlusBTree.h>
#include <HFSCatalog.h>

void GetNextBlockNum(uint64_t* _nodeBlockNumber, uint64_t* _extentNum, uint64_t* _currentBlockNum,
                     BTNodeDescriptor descriptor, FlexCommanderFS fs);
void ReadNodeDescriptor(FlexCommanderFS fs, uint64_t nodeBlockNumber, BTNodeDescriptor *descriptor, char *rawNode);
void FillRecordAddress(BTHeaderRec btreeHeader, BTNodeDescriptor descriptor, char *rawNode, uint16_t *recordAddress);
char *HFSStringToBytes(HFSUniStr255 hfsStr);
bool HFSStrToStrCmp(HFSUniStr255 hfsStr, const char *str);
bool CheckForHFSPrivateDataNode(HFSPlusCatalogKey key);
void PrintHFSUnicode(HFSUniStr255 str);

// Use only with structures. Idk why, but it doesn't work with 4 byte values.
#define CAST_PTR_TO_TYPE(type, ptr) *(type*)ptr
