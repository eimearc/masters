#include "bench.h"
#include <iostream>
#include "triangle.h"

template<typename T>
void runBench(GLFWwindow *window, std::string fileName)
{
    Bench bench;
    bench.open(fileName);

    time_point startupTime, frameTime;
    for (size_t t = 1; t <= 4; ++t)
    {
        bench.numThreads(t);
        for (size_t i = 0; i<10; ++i)
        {
            startupTime = bench.start();
            T tb(window,t);
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
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow *window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    runBench<TriangleBench>(window, "triangle.txt");
}