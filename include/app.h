#ifndef APP
#define APP

#include "evulkan_core.h"

#include "flags.h"
#include "grid.h"

class App {
public:
    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    void run()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
        
        if (FLAGS_multipass) multipass();
        else basic();
    }

    void basic()
    {
        createGrid();
        initVulkan();
        mainLoop();
        texture.destroy();
        destroy();
    }

    void multipass()
    {
        createGrid();
        initMultipassVulkan();
        mainLoop();
        destroy();
    }

    void destroy()
    {
        buffer.destroy();
        indexBuffer.destroy();
        vertexBuffer.destroy();
        for (auto &a : attachments) a.destroy();
        framebuffers.destroy();
        swapchain.destroy();
        commands.destroy();
        for (auto &d : descriptors) d.destroy();
        for (auto &p : pipelines) p.destroy();
        for (auto &s : shaders) s.destroy();
        renderpass.destroy();
        sync.destroy();
        device.destroy(); 
    }

~App()=default;

private:
    const size_t NUM_CUBES = sqrt(FLAGS_num_cubes);
    const int MAX_FRAMES_IN_FLIGHT = 2;

    Grid grid;
    
    Device device;
    Buffer buffer;
    Buffer indexBuffer;
    Buffer vertexBuffer;
    Renderpass renderpass;
    std::vector<Attachment> attachments;
    std::vector<Descriptor> descriptors; // TODO: make this a vector of pointers.
    Texture texture;
    Swapchain swapchain;
    Commands commands;
    Framebuffer framebuffers;
    Sync sync;
    std::vector<Pipeline> pipelines;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<Shader> shaders;
    GLFWwindow *window;
    std::vector<const char*> validationLayers =
    {
        "VK_LAYER_LUNARG_standard_validation"
    };
    std::vector<const char*> deviceExtensions = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    void createGrid();
    void initVulkan();
    void initMultipassVulkan();
    void mainLoop();
    void cleanup();
};

#endif
