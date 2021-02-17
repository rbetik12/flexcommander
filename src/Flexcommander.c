#include <Flexcommander.h>

int main() {
    FlexCommanderFS fs;
    FlexGetDirElements("/mnt/temp", &fs);
    return 0;
}
