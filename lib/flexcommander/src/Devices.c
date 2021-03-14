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

    while (blkid_dev_next(iterator, &device) == 0) {
        const char * devName = blkid_dev_devname(device);
        printf("%s", devName);

        if (isRoot) {
            blkid_probe probe = blkid_new_probe_from_filename(devName);
            if (probe == NULL) {
                fprintf(stderr, "Launch util as root to get more information!\n");
                isRoot = false;
            }
            else {
                blkid_loff_t probeSize = blkid_probe_get_size(probe);
                printf("\t");
                if (probeSize >= gibibyteDivider) {
                    printf("%lld GiB\t", (long long) (probeSize / gibibyteDivider));
                } else if (probeSize < gibibyteDivider) {
                    printf("%lld MiB\t", (long long) (probeSize / mibibyteDivider));
                }
                blkid_do_probe(probe);
                const char * fsType;
                blkid_probe_lookup_value(probe, "TYPE", &fsType, NULL);
                printf("%s\n", fsType);
            }

        }
    }

    blkid_dev_iterate_end(iterator);
    return 0;
}