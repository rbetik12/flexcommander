#include <Flexcommander.h>

int main() {
    FlexCommanderFS fs;
    if (FlexOpen("/home/vitaliy/itmo/spo/lab1/res/fs/hfs.img", &fs)) {
        fprintf(stderr, "Error!\n");
    }

    FlexListDirContent("/", &fs);

    return 0;
}