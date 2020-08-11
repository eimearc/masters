#include "bench.h"
#include "multipass.h"
#include "obj.h"
#include "triangle.h"

const size_t NUM_SETUPS = 10;
const size_t NUM_FRAMES = 10;

template<typename T>
void runBench(GLFWwindow *window, std::string fileName)
{
    Bench bench;
    bench.open(fileName);

    time_point startupTime, frameTime;
    for (size_t t = 1; t <= 4; ++t)
    {
        bench.numThreads(t);
        for (size_t i = 0; i<NUM_SETUPS; ++i)
        {
            startupTime = bench.start();
            T tb(window,t);
            bench.startupTime(startupTime);
            bench.numVerts(tb.numVerts());
            for (size_t j = 0; j<NUM_FRAMES; ++j)
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
    runBench<MultipassBench>(window, "multipass.txt");
    runBench<ObjBench>(window, "obj.txt");
}