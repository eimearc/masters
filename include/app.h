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

    void run() {
        createGrid();
        initVulkan();
        mainLoop();
        evkInstance.cleanup();
    }

~App()=default;

private:
    const size_t NUM_CUBES = sqrt(FLAGS_num_cubes);
    const int MAX_FRAMES_IN_FLIGHT = 2;

    Grid grid;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    evk::Instance evkInstance;
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
    void mainLoop();
    void cleanup();
};
