#include "evulkan.h"

#include <iostream>

DEFINE_bool(enable_validation,false,"Turn on validation layers for Vulkan.");

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program for benchmarking Vulkan over multiple threads.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (FLAGS_enable_validation) std::cout << "Validation layers turned on. Turn off for better performance.\n";

    EVulkan app;

    try
    {
        app.run();
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // exit(0); // TODO: figure out why this doesn't work.
    return EXIT_SUCCESS;
}
