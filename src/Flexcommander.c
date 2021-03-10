#include <Flexcommander.h>

int main() {
    FlexCommanderFS fs;
    if (FlexOpen("/home/vitaliy/itmo/spo/flexcommander/res/fs/hfs1.img", &fs)) {
        fprintf(stderr, "Error!\n");
    }

    FlexListDirContent("/lol", &fs);

    return 0;
}