#include <Flexcommander.h>
#include <stddef.h>
#include <stdio.h>

int Init(FlexCommanderProbeInfo* info) {
    int status = blkid_get_cache(&info->blkCache, NULL);
    if (status < 0) {
        fprintf(stderr, "Can't initialize blkid lib!\n");
        return -1;
    }
    return 0;
}

