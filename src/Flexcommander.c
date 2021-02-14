#include <Flexcommander.h>


int main() {
    const char * path = "/home/vitaliy/itmo/spo/lab1/res/fs/hfs.img";

    FlexCommanderFS fs;
    FlexOpenAndMount(path, &fs);

    return 0;
}
