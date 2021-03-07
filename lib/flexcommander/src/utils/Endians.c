#include "Endians.h"
#include <byteswap.h>
#include <netinet/in.h>

void ConvertBTreeHeader(BTHeaderRec* header) {
    header->treeDepth = bswap_16(header->treeDepth);
    header->rootNode = htonl(header->rootNode);
    header->leafRecords = htonl(header->leafRecords);
    header->firstLeafNode = htonl(header->firstLeafNode);
    header->lastLeafNode = htonl(header->lastLeafNode);
    header->nodeSize = bswap_16(header->nodeSize);
    header->maxKeyLength = bswap_16(header->maxKeyLength);
    header->totalNodes = htonl(header->totalNodes);
    header->freeNodes = htonl(header->freeNodes);
    header->clumpSize = htonl(header->clumpSize);
}

void ConvertBTreeNodeDescriptor(BTNodeDescriptor* descr) {
    descr->fLink = bswap_32(descr->fLink);
    descr->bLink = bswap_32(descr->bLink);
    descr->numRecords = bswap_16(descr->numRecords);
}

void ConvertCatalogKey(HFSPlusCatalogKey* key) {
    key->keyLength = bswap_16(key->keyLength);
    key->parentID = bswap_32(key->parentID);
    key->nodeName.length = bswap_16(key->nodeName.length);
    for (int i = 0; i < key->nodeName.length; i++) {
        key->nodeName.unicode[i] = bswap_16(key->nodeName.unicode[i]);
    }
}

void ConvertCatalogFolder(HFSPlusCatalogFolder* folder) {
    folder->recordType = bswap_16(folder->recordType);
    folder->flags = bswap_16(folder->flags);
    folder->valence = bswap_32(folder->valence);
    folder->folderID = bswap_32(folder->folderID);
    folder->createDate = bswap_32(folder->createDate);
    folder->contentModDate = bswap_32(folder->contentModDate);
    folder->attributeModDate = bswap_32(folder->attributeModDate);
    folder->accessDate = bswap_32(folder->accessDate);
    folder->backupDate = bswap_32(folder->backupDate);
    folder->textEncoding = bswap_32(folder->textEncoding);
}