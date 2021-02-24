#pragma once

#include <stdio.h>
#include <blkid/blkid.h>

typedef struct FlexCommanderProbeInfo {
    blkid_cache blkCache;
} FlexCommanderProbeInfo;

typedef struct FlexCommanderFS {
    FILE *file;
    uint32_t blockSize;
} FlexCommanderFS;

//Functions for block device probing
int Init(FlexCommanderProbeInfo* info);
int ProbeDevices(FlexCommanderProbeInfo* info);
int IterateDevices(FlexCommanderProbeInfo* info);

//Functions for filesystem interaction
int FlexOpen(const char * path, FlexCommanderFS* fs);