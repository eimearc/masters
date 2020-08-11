#include "bench.h"
#include <iostream>
#include "multipass.h"
#include "obj.h"
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
            bench.numVerts(tb.numVerts());
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

    const std::string dir = "timings/"; // This needs to be changed for Windows.

    runBench<TriangleBench>(window, dir + "triangle.txt");
    runBench<MultipassBench>(window, dir + "multipass.txt");
    runBench<ObjBench>(window, dir + "obj.txt");
}