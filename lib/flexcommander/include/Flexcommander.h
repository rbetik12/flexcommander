#pragma once

#include <stdio.h>
#include <blkid/blkid.h>
#include "HFSPlus.h"

typedef struct FlexCommanderProbeInfo {
    blkid_cache blkCache;
} FlexCommanderProbeInfo;

typedef struct FlexCommanderFS {
    FILE *file;
    uint32_t blockSize;
    uint64_t catalogFileBlock;
    HFSPlusVolumeHeader volumeHeader;
} FlexCommanderFS;

//Functions for block device probing
int Init(FlexCommanderProbeInfo* info);
int ProbeDevices(FlexCommanderProbeInfo* info);
int IterateDevices(FlexCommanderProbeInfo* info);

//Functions for filesystem interaction
int FlexOpen(const char * path, FlexCommanderFS* fs);
int FlexListDirContent(const char* path, FlexCommanderFS* fs);
int FlexSetCurrentDir(const char* path, FlexCommanderFS* fs);
int FlexCopy(const char* path, const char* currentDir, FlexCommanderFS* fs);