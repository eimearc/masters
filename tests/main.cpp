#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

int main()
{
    testing::InitGoogleTest();

    auto result = RUN_ALL_TESTS();

    return result;
}