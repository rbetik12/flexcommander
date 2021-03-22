#pragma once

#include <HFSCatalog.h>
#include "HFSPlusBTree.h"

void ConvertBTreeHeader(BTHeaderRec* header);
void ConvertBTreeNodeDescriptor(BTNodeDescriptor* descr);
void ConvertCatalogKey(HFSPlusCatalogKey* key);
void ConvertCatalogFolder(HFSPlusCatalogFolder* folder);
void ConvertCatalogFile(HFSPlusCatalogFile* file);
void ConvertCatalogThread(HFSPlusCatalogThread* catalogThread);
void ConvertCatalogIndexNode(BTCatalogIndexNode *indexNode);
void ConvertBSDInfo(HFSPlusBSDInfo* info);