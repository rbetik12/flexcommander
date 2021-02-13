#pragma once
#include <blkid/blkid.h>

typedef struct FlexCommanderInfo {
    blkid_cache blkCache;
} FlexCommanderInfo;

void Hello();

int Init(FlexCommanderInfo* info);
int ProbeDevices(FlexCommanderInfo* info);
int IterateDevices(FlexCommanderInfo* info);
