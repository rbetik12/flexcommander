#include "HFSPlusBTree.h"
#include <FlexIO.h>
#include "utils/Endians.h"

void ParseNode(uint64_t nodeBlock, BTHeaderRec btreeHeader, FlexCommanderFS fs) {
    uint64_t nodeBlockAddress = nodeBlock * fs.blockSize;
    FlexFSeek(fs.file, nodeBlockAddress, SEEK_SET);

    BTNodeDescriptor nodeDescr;
    FlexRead(&nodeDescr, sizeof(BTNodeDescriptor), 1, fs.file);

    ConvertBTreeNodeDescriptor(&nodeDescr);
    PrintBTreeNodeDescriptor(nodeDescr);
}