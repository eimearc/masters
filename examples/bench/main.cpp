#include "bench.h"
#include "multipass.h"
#include "obj.h"
#include "triangle.h"
#include "simple_triangle.h"

const size_t NUM_SETUPS = 100;
const size_t NUM_FRAMES = 100;

template<typename T>
void runBench(GLFWwindow *window, std::string fileName)
{
    Bench bench;
    bench.open(fileName);

    printf("\n\n\n** %s **\n", fileName.c_str());
    time_point startupTime, frameTime;
    for (size_t t = 1; t <= 4; ++t)
    {
        printf("Running threads: %zu\n", t);
        bench.numThreads(t);
        for (size_t i = 0; i<NUM_SETUPS; ++i)
        {
            printf("\tRunning setup: %zu\n", i);
            startupTime = bench.start();
            T tb(window,t);
            bench.startupTime(startupTime);
            bench.numVerts(tb.numVerts());
            printf("\t\tRunning frames: ");
            for (size_t j = 0; j<NUM_FRAMES; ++j)
            {
                printf("%zu ", j);
                glfwPollEvents();
                frameTime = bench.start();
                tb.draw();
                bench.frameTime(frameTime);
                bench.record();
            }
            std::cout << "\n";
        }
    } 
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow *window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    runBench<TriangleBench>(window, "triangle.csv");
    runBench<MultipassBench>(window, "multipass.csv");
    runBench<ObjBench>(window, "obj.csv");
    runBench<SimpleTriangleBench>(window, "simple_triangle.csv");
}