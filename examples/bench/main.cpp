#include "bench.h"
#include <iostream>
#include "triangle.h"

int main()
{
    Bench bench;
    bench.open("triange.txt");

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow *window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    time_point startupTime, frameTime;
    for (size_t i = 0; i<10; ++i)
    {
        startupTime = bench.start();
        TriangleBench tb(window);
        bench.startupTime(startupTime);
        for (size_t j = 0; j<10; ++j)
        {
            glfwPollEvents();
            frameTime = bench.start();
            tb.draw();
            bench.frameTime(frameTime);
            bench.record();
        }
    }
}