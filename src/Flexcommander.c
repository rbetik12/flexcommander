#include <Flexcommander.h>

int main() {
    FlexCommanderFS fs;
    if (FlexOpen("/home/vitaliy/itmo/spo/flexcommander/res/fs/hfs.img", &fs)) {
        fprintf(stderr, "Error!\n");
    }

    FlexListDirContent("/lol", &fs);

    return 0;
}