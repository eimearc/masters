#include "app.h"

#include <iostream>

DECLARE_bool(multipass);

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program for benchmarking Vulkan over multiple threads.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // if (FLAGS_enable_validation) std::cout << "Validation layers turned on. Turn off for better performance.\n";

    App app;

    try
    {
        app.run();
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
