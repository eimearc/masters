#pragma once

#include "evulkan_core.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>
#include <set>
#include <fstream>
#include <array>
#include <chrono>

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
        if (!FLAGS_multipass) basic();
        if (FLAGS_multipass) multipass();
    }

    void basic()
    {
        createGrid();
        initVulkan();
        mainLoop(evkInstance);
        evkInstance.cleanup();
        texture.destroy();
        destroy();
    }

    void multipass()
    {
        createGrid();
        initMultipassVulkan();
        mainLoop(multipassInstance);
        multipassInstance.cleanup();
        destroy();
    }

    void destroy()
    {
        buffer.destroy();
        indexBuffer.destroy();
        vertexBuffer.destroy();
        for (auto &a : attachments) a.destroy();
        swapchain.destroy();
        commands.destroy();
        for (auto &d : descriptors) d.destroy();
        for (auto &p : pipelines) p.destroy();
        for (auto &s : shaders) s.destroy();
        renderpass.destroy();
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
    std::vector<Pipeline> pipelines;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    evk::Instance evkInstance;
    evk::Instance multipassInstance;
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
    void mainLoop(evk::Instance &instance);
    void cleanup();
};
