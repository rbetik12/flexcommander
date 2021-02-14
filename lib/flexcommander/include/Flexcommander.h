#pragma once
#include <blkid/blkid.h>
#include <stdio.h>

typedef struct FlexCommanderProbeInfo {
    blkid_cache blkCache;
} FlexCommanderProbeInfo;

typedef struct FlexCommanderFS {
    FILE *file;
} FlexCommanderFS;

//Functions for block device probing
int Init(FlexCommanderProbeInfo* info);
int ProbeDevices(FlexCommanderProbeInfo* info);
int IterateDevices(FlexCommanderProbeInfo* info);

//Functions for filesystem interaction
int FlexOpenAndMount(const char * path, FlexCommanderFS* fs);
int FlexVerify(FlexCommanderFS* fs);