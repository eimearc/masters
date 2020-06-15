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

    commandPools.resize(FLAGS_num_threads);
    for (auto &cp : commandPools)
    {
        EVkCommandPoolCreateInfo info = {};
        info.physicalDevice = physicalDevice;
        info.surface = surface;
        evkCreateCommandPool(device, &info, &cp);
    }
    evkInstance.m_commandPools=commandPools;

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

    threadPool.setThreadCount(FLAGS_num_threads);

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

void EVulkan::cleanup()
{
    EVkSwapchainCleanupInfo cleanupInfo = {};
    cleanupInfo.depthImage = depthImage;
    cleanupInfo.depthImageView = depthImageView;
    cleanupInfo.depthImageMemory = depthImageMemory;
    cleanupInfo.swapchainFramebuffers = swapChainFramebuffers;
    cleanupInfo.graphicsPipeline = graphicsPipeline;
    cleanupInfo.pipelineLayout = pipelineLayout;
    cleanupInfo.renderPass = renderPass;
    cleanupInfo.swapchainImageViews = swapChainImageViews;
    cleanupInfo.swapchain = swapChain;
    cleanupInfo.uniformBuffers = uniformBuffers;
    cleanupInfo.uniformBuffersMemory = uniformBuffersMemory;
    cleanupInfo.descriptorPool = descriptorPool;
    cleanupInfo.swapchainImages = swapChainImages;
    evkCleanupSwapchain(device, &cleanupInfo);

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    for (int i = 0; i < commandPools.size(); ++i)
    {
        vkFreeCommandBuffers(device, commandPools[i], 1, &secondaryCommandBuffers[i]);
        vkDestroyCommandPool(device, commandPools[i], nullptr);
    }

    vkDestroyDevice(device, nullptr);

    if (validationLayers.size() > 0)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}