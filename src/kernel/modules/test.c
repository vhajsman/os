#include "interface.h"
#include "console.h"

void interface_test_init(struct kernel_interface_callflags* callflags) {
    callflags->exit_code = 0;
}

void interface_test_deinit(struct kernel_interface_callflags* callflags) {
    callflags->exit_code = 0;
}

void interface_test() {
    struct kernel_interface myInterface;
    myInterface.name_short = "test";
    myInterface.name_friendly = "My testing interface";
    myInterface.init = &interface_test_init;
    myInterface.deinit = &interface_test_deinit;

    struct kernel_interface_callflags myCallFlags;
    myCallFlags.flags = 0;

    kernel_interface_load(-1, &myInterface);
    int idx = kernel_interface_findByName("test");

    kernel_interface_init(idx, &myCallFlags);
    kernel_interface_deinit(idx, &myCallFlags);

    kernel_interface_unload(idx);
}