#include "device.h"

#include <gtest/gtest.h>

TEST(Device, firstTest)
{
    std::cout << "Hello from first test.\n";
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}