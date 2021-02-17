#ifdef HFSPlus

#include <Flexcommander.h>
#include <HFSPlus.h>
#include <errno.h>
#include <sys/mount.h>
#include <string.h>
#include <fcntl.h>
#include <linux/loop.h>
#include <zconf.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define DATE_BUFFER_SIZE 64

int SetupLoopDevice(char* deviceName, FILE* file);

int Mount(const char* path, FlexCommanderFS* fs);

void FormatDate(char* buffer, time_t value);

void PrintMode(struct stat fileStat);

int FlexOpenAndMount(const char* path, FlexCommanderFS* fs) {
    FILE* hfs = fopen(path, "rb+");
    fs->mountedDirName = "/mnt/temp";

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

int FlexGetDirElements(const char* path, FlexCommanderFS* fs) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir) {
        perror("diropen");
        return -1;
    };

    printf("Name        Type        Size        Modified        Mode\n");
    char stringBuffer[4096];
    while ( (entry = readdir(dir)) != NULL) {
        printf("%s      ", entry->d_name);
        switch (entry->d_type) {
            case DT_UNKNOWN:
                printf("%s      ", "Unknown");
                break;
            case DT_REG:
                printf("%s      ", "File");
                break;
            case DT_DIR:
                printf("%s      ", "Directory");
                break;
        }

        const char* fullFilePath = strcat(stringBuffer, path);
        fullFilePath = strcat(fullFilePath, "/");
        fullFilePath = strcat(fullFilePath, entry->d_name);

        struct stat statBuffer;
        if (stat(fullFilePath, &statBuffer)) {
            perror("Can't get file stats!\n");
            memset(fullFilePath, 0, sizeof(stringBuffer));
            printf("-1      -1\n");
            break;
        }

        printf("%ld     ", statBuffer.st_size);
        char date[DATE_BUFFER_SIZE] = {0};
        FormatDate(date, statBuffer.st_mtim.tv_sec);
        printf("        %s      ", date);
        PrintMode(statBuffer);
        printf("\n");
        memset(fullFilePath, 0, sizeof(stringBuffer));
    };

    closedir(dir);
    return 0;
}

void FormatDate(char* buffer, time_t value) {
    strftime(buffer, DATE_BUFFER_SIZE, "%d.%m.%Y %H:%M:%S", localtime(&value));
}

void PrintMode(struct stat fileStat) {
    //Took that from stackoverflow, cause I am too lazy for doing that by myself. I am sorry :(
    printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
}
#endif

