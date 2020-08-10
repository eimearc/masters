#ifndef EVK_EXAMPLES_UTIL_H_
#define EVK_EXAMPLES_UTIL_H_

#include "evulkan.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

std::vector<const char*> validationLayers =
{
    "VK_LAYER_LUNARG_standard_validation"
};
std::vector<const char*> deviceExtensions = 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

std::vector<evk::Vertex> setupVerts()
{
    std::vector<evk::Vertex> verts;
    evk::Vertex v;
    v.pos={0,-0.5,0};
    v.color={1,0,0};
    verts.push_back(v);
    v.pos={-0.5,0.5,0};
    v.color={0,0,1};
    verts.push_back(v);
    v.pos={0.5,0.5,0};
    v.color={0,1,0};
    verts.push_back(v);
    return verts;
}

std::vector<const char*> glfwExtensions()
{
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> surfaceExtensions(
        glfwExtensions, glfwExtensions + glfwExtensionCount
    );
    return surfaceExtensions;
}

struct WindowResize
{
    static void resizeGLFW(GLFWwindow *window, int width, int height)
    {
        auto r = reinterpret_cast<WindowResize*>(
            glfwGetWindowUserPointer(window)
        );
        r->device->framebufferOutofDate = true;
    }
    evk::Device *device;
};

void createSurfaceGLFW(evk::Device &device, GLFWwindow *window, WindowResize &r)
{
    r = {&device};
    glfwSetWindowUserPointer(window,&r);

    auto surfaceExtensions = glfwExtensions();
    auto surfaceFunc = [&](){
        glfwCreateWindowSurface(
            device.instance(), window, nullptr, &device.surface()
        );
    };

    device.createSurface(surfaceFunc,800,600,surfaceExtensions);
    glfwSetFramebufferSizeCallback(window, r.resizeGLFW);
}

#endif