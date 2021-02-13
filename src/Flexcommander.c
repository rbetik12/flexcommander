#include <Flexcommander.h>


int main() {
    FlexCommanderInfo info;
    Init(&info);
    ProbeDevices(&info);
    IterateDevices(&info);

    return 0;
}
