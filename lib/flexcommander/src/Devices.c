#include <Flexcommander.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

bool isRoot = true;

int ProbeDevices(FlexCommanderProbeInfo* info) {
    int status = blkid_probe_all(info->blkCache);

    if (status < 0){
        fprintf(stderr, "Can't probe devices!\n");
        return -1;
    }
    return 0;
}

int IterateDevices(FlexCommanderProbeInfo* info) {
    blkid_dev device;
    blkid_dev_iterate iterator = blkid_dev_iterate_begin(info->blkCache);
    const double gibibyteDivider = pow(2, 30);
    const double mibibyteDivider = pow(2, 20);

    printf("Partitions\n");
    while (blkid_dev_next(iterator, &device) == 0) {
        const char * devName = blkid_dev_devname(device);
        printf("\t%s\n", devName);

        if (isRoot) {
            blkid_probe probe = blkid_new_probe_from_filename(devName);
            if (probe == NULL) {
                fprintf(stderr, "Launch util as root to get more information!\n");
                isRoot = false;
            }
            else {
                blkid_loff_t probeSize = blkid_probe_get_size(probe);
                printf("\t\tSize: ");
                if (probeSize >= gibibyteDivider) {
                    printf("%lld GiB\n", (long long) (probeSize / gibibyteDivider));
                } else if (probeSize < gibibyteDivider) {
                    printf("%lld MiB\n", (long long) (probeSize / mibibyteDivider));
                }
            }

        }
    }

    blkid_dev_iterate_end(iterator);
    return 0;
}