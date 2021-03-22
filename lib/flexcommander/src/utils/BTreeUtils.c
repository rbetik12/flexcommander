#include "BTreeUtils.h"
#include <string.h>
#include <stdlib.h>
#include <FlexIO.h>
#include "Endians.h"

void GetNextBlockNum(uint64_t* _nodeBlockNumber, uint64_t* _extentNum, uint64_t* _currentBlockNum,
                     BTNodeDescriptor descriptor, FlexCommanderFS fs) {
    uint64_t nodeBlockNumber = *_nodeBlockNumber;
    uint64_t extentNum = *_extentNum;
    uint64_t currentBlockNum = *_currentBlockNum;

    if (nodeBlockNumber == fs.volumeHeader.catalogFile.extents[extentNum].startBlock +
                           fs.volumeHeader.catalogFile.extents[extentNum].blockCount - 1) {
        extentNum += 1;
        nodeBlockNumber = fs.volumeHeader.catalogFile.extents[extentNum].startBlock;
    } else {
        nodeBlockNumber += descriptor.fLink - currentBlockNum;
        if (nodeBlockNumber >= fs.volumeHeader.catalogFile.extents[extentNum].startBlock +
                               fs.volumeHeader.catalogFile.extents[extentNum].blockCount) {
            extentNum += 1;
            nodeBlockNumber = fs.volumeHeader.catalogFile.extents[extentNum].startBlock;
        }
    }
    currentBlockNum = descriptor.fLink;

    *_nodeBlockNumber = nodeBlockNumber;
    *_extentNum = extentNum;
    *_currentBlockNum = currentBlockNum;
}

void ReadNodeDescriptor(FlexCommanderFS fs, uint64_t nodeBlockNumber, BTNodeDescriptor *descriptor, char *rawNode) {
    FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
    FlexRead(rawNode, fs.blockSize, 1, fs.file);
    descriptor = memcpy(descriptor, &CAST_PTR_TO_TYPE(BTNodeDescriptor, rawNode), sizeof(BTNodeDescriptor));
    ConvertBTreeNodeDescriptor(descriptor);
}

void FillRecordAddress(BTHeaderRec btreeHeader, BTNodeDescriptor descriptor, char *rawNode, uint16_t *recordAddress) {
    int j = 0;
    for (int i = btreeHeader.nodeSize - 1; i >= btreeHeader.nodeSize - descriptor.numRecords * 2; i -= 2) {
        recordAddress[j] = (rawNode[i - 1] << 8) | (uint8_t) rawNode[i];
        j += 1;
    }
}

char *HFSStringToBytes(HFSUniStr255 hfsStr) {
    char *str = calloc(hfsStr.length + 1, 1);
    for (int i = 0; i < hfsStr.length; i++) {
        str[i] = hfsStr.unicode[i];
    }
    return str;
}

bool HFSStrToStrCmp(HFSUniStr255 hfsStr, const char *str) {
    size_t strLen = strlen(str);
    if (strLen != hfsStr.length) {
        return false;
    }

    int i = 0;
    for (i = 0; i < hfsStr.length; i++) {
        if (hfsStr.unicode[i] != str[i]) {
            return false;
        }
    }

    return true;
}

//Checks whether given node is HFS+ Private Data node
bool CheckForHFSPrivateDataNode(HFSPlusCatalogKey key) {
    if (key.nodeName.length != 21) return false;
    uint16_t checkSymbols[21] = {0, 0, 0, 0, 72, 70, 83, 43, 32, 80, 114, 105, 118, 97, 116, 101, 32, 68, 97, 116, 97};
    for (int i = 0; i < 21; i++) {
        if (key.nodeName.unicode[i] != checkSymbols[i]) return false;
    }
    return true;
}

void PrintHFSUnicode(HFSUniStr255 str) {
    for (int i = 0; i < str.length; i++) {
        printf("%lc", str.unicode[i]);
    }
}