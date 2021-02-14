#ifdef HFSPlus

#include <Flexcommander.h>
#include <HFSPlus.h>
#include <errno.h>
#include <sys/mount.h>
#include <string.h>
#include <fcntl.h>
#include <linux/loop.h>
#include <zconf.h>

int SetupLoopDevice(char* deviceName, FILE* file);

int Mount(const char* path, FlexCommanderFS* fs);

int FlexOpenAndMount(const char* path, FlexCommanderFS* fs) {
    FILE* hfs = fopen(path, "rb+");

    if (hfs == NULL) {
        fprintf(stderr, "Can't open file system at %s\n", path);
        fprintf(stderr, "Error code: %d", errno);
        return -1;
    }

    fs->file = hfs;

    if (FlexVerify(fs)) {
        return -1;
    }

    if (Mount(path, fs)) {
        return -1;
    }
    return 0;
}

int FlexVerify(FlexCommanderFS* fs) {
    HFSPlusVolumeHeader header;

    if (fseek(fs->file, HFS_START_OFFSET, SEEK_SET)) {
        fprintf(stderr, "Can't set 1024 bytes offset!\n");
        return -1;
    }

    if (fread(&header, sizeof(HFSPlusVolumeHeader), 1, fs->file) != 1) {
        if (feof(fs->file)) {
            fprintf(stderr, "Unexpected EOF!\n");
        } else {
            fprintf(stderr, "Can't read HFS volume header!\n");
        }
        return -1;
    }

    if (header.signature == HFS_SIGNATURE) {
        return 0;
    } else {
        fprintf(stderr, "Provided file is not a HFS volume!\n");
        return -1;
    }
}

int Mount(const char* path, FlexCommanderFS* fs) {
    char loopDevicePath[1024] = {0};
    if (SetupLoopDevice(loopDevicePath, fs->file)) {
        fprintf(stderr, "Can't setup loop device!\n");
        return -1;
    }

    if (mount(loopDevicePath, "/mnt/temp", "hfsplus", 0, "")) {
        fprintf(stderr, "Can't mount fs!\n");
        switch (errno) {
            case EPERM:
                fprintf(stderr, "Try again as root!\n");
                break;
            case ENOENT:
                fprintf(stderr, "Can't locate file or mounting target!\n");
                break;
            default:
                fprintf(stderr, "%s\n", strerror(errno));
                fprintf(stderr, "%d\n", errno);
                break;
        }
        return -1;
    }
    return 0;
}

int SetupLoopDevice(char* deviceName, FILE* file) {
    int loopControlDeviceDescriptor;
    long loopDeviceNumber;
    loopControlDeviceDescriptor = open("/dev/loop-control", O_RDWR);
    loopDeviceNumber = ioctl(loopControlDeviceDescriptor, LOOP_CTL_GET_FREE);
    sprintf(deviceName, "/dev/loop%ld", loopDeviceNumber);

    int loopDeviceDescriptor;
    if ((loopDeviceDescriptor = open(deviceName, O_RDWR)) < 0) {
        fprintf(stderr, "Failed to open device (%s).\n", deviceName);
        return -1;
    }

    int fsDescriptor = fileno(file);
    if (ioctl(loopDeviceDescriptor, LOOP_SET_FD, fsDescriptor) < 0) {
        fprintf(stderr, "Failed to set fd.\n");
        return -1;
    }

    struct loop_info64 info;
    memset(&info, 0, sizeof(struct loop_info64));
    info.lo_offset = 0;
    close(fsDescriptor);
    if (ioctl(loopDeviceDescriptor, LOOP_SET_STATUS64, &info)) {
        fprintf(stderr, "Failed to set info.\n");
        return -1;
    }

    close(loopDeviceDescriptor);
    return 0;
}


#endif

