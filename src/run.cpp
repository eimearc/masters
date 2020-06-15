#include "evulkan.h"

void EVulkan::initVulkan()
{
    evkCreateWindow(EVkCreateWindow{}, window);

    evk::InstanceCreateInfo instanceCreateInfo{
        validationLayers,
        window,
        deviceExtensions
    };

    evkInstance=evk::Instance(FLAGS_num_threads, &instanceCreateInfo);
    
    instance=evkInstance.m_vkInstance;
    debugMessenger=evkInstance.m_debugMessenger;
    surface=evkInstance.m_surface;
    physicalDevice=evkInstance.m_physicalDevice;
    graphicsQueue=evkInstance.m_graphicsQueue;
    presentQueue=evkInstance.m_presentQueue;
    device=evkInstance.m_device;

    evkInstance.createCommandPools();
    commandPools=evkInstance.m_commandPools;

    evk::SwapChainCreateInfo swapChainCreateInfo{
        static_cast<uint8_t>(MAX_FRAMES_IN_FLIGHT)
    };
    evkInstance.createSwapChain(&swapChainCreateInfo);
    swapChain=evkInstance.m_swapChain;
    swapChainImages=evkInstance.m_swapChainImages;
    swapChainImageFormat=evkInstance.m_swapChainImageFormat;
    swapChainExtent=evkInstance.m_swapChainExtent;
    swapChainImageViews=evkInstance.m_swapChainImageViews;

    evkInstance.createSyncObjects();
    imageAvailableSemaphores=evkInstance.m_imageAvailableSemaphores;
    renderFinishedSemaphores=evkInstance.m_renderFinishedSemaphores;
    inFlightFences=evkInstance.m_fencesInFlight;
    imagesInFlight=evkInstance.m_imagesInFlight;

    evkInstance.createRenderPass();
    renderPass=evkInstance.m_renderPass;

    // ----------------
    // Below are linked
    evkInstance.createUniformBufferObject();
    uniformBuffers=evkInstance.m_uniformBuffers;
    uniformBuffersMemory=evkInstance.m_uniformBuffersMemory;

    evkInstance.createDescriptorSets();
    descriptorPool=evkInstance.m_descriptorPool;
    descriptorSetLayout=evkInstance.m_descriptorSetLayout;
    descriptorSets=evkInstance.m_descriptorSets;
    // ----------------

    evk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
        "shaders/vert.spv",
        "shaders/frag.spv"
    };
    evkInstance.createGraphicsPipeline(&graphicsPipelineCreateInfo);
    graphicsPipeline=evkInstance.m_graphicsPipeline;
    pipelineLayout=evkInstance.m_graphicsPipelineLayout;

    evkInstance.createDepthResources();
    depthImage=evkInstance.m_depthImage;
    depthImageView=evkInstance.m_depthImageView;
    depthImageMemory=evkInstance.m_depthImageMemory;

    evkInstance.createFramebuffers();
    swapChainFramebuffers=evkInstance.m_framebuffers;

    evkInstance.createIndexBuffer(indices);
    indexBuffer=evkInstance.m_indexBuffer;
    indexBufferMemory=evkInstance.m_indexBufferMemory;

    evkInstance.createVertexBuffer(vertices);
    vertexBuffer=evkInstance.m_vertexBuffer;
    vertexBufferMemory=evkInstance.m_vertexBufferMemory;

    evkInstance.createDrawCommands(indices);

    secondaryCommandBuffers=evkInstance.m_secondaryCommandBuffers;
    primaryCommandBuffers=evkInstance.m_primaryCommandBuffers;
}

void EVulkan::mainLoop()
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        evkInstance.draw();
    }

    if (vkDeviceWaitIdle(device)!=VK_SUCCESS)
    {
        throw std::runtime_error("Could not wait for vkDeviceWaitIdle");
    }
}

void evk::Instance::cleanup()
{
    vkDestroyImageView(m_device, m_depthImageView, nullptr);
    vkDestroyImage(m_device, m_depthImage, nullptr);
    vkFreeMemory(m_device, m_depthImageMemory, nullptr);

    for (auto framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_graphicsPipelineLayout, nullptr);
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);

    for (auto imageView : m_swapChainImageViews)
    {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

    for (size_t i = 0; i < m_swapChainImages.size(); i++)
    {
        vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
        vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

    vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
    vkFreeMemory(m_device, m_indexBufferMemory, nullptr);

    vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);

    for (size_t i = 0; i < m_maxFramesInFlight; ++i)
    {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_fencesInFlight[i], nullptr);
    }

    for (int i = 0; i < m_commandPools.size(); ++i)
    {
        vkFreeCommandBuffers(m_device, m_commandPools[i], 1, &m_secondaryCommandBuffers[i]);
        vkDestroyCommandPool(m_device, m_commandPools[i], nullptr);
    }

    vkDestroyDevice(m_device, nullptr);

    if (m_validationLayers.size() > 0)
    {
        DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
    vkDestroyInstance(m_vkInstance, nullptr);

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void EVulkan::cleanup()
{
    evkInstance.cleanup();
}