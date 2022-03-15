#include <iostream>
#include <btlib.h>
#include <fmt/core.h>

int main() {
    if (init_blue("device.txt") == 0) {
        return 0;
    }

    classic_scan();

    int res = device_info(BTYPE_CL);

    fmt::print("device info returned {} \n", res);

    close_all();
    return 0;
}
