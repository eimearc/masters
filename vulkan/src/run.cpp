#include "evulkan.h"

void EVulkan::initVulkan()
{
    evkCreateWindow(EVkCreateWindow{}, window);

    EVkCreateInstance instanceInfo = {};
    instanceInfo.appTitle = "Vulkan App";
    instanceInfo.extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    instanceInfo.validationLayers = validationLayers;
    evkCreateInstance(&instanceInfo, &instance);

    evkSetupDebugMessenger(instance, &debugMessenger);

    EVkSurfaceCreate surfaceInfo{};
    surfaceInfo.window = window;
    evkCreateSurface(instance, &surfaceInfo, &surface);
    
    EVkPickPhysicalDevice pickInfo = {};
    pickInfo.surface = surface;
    evkPickPhysicalDevice(instance, &pickInfo, &physicalDevice);

    EVkDeviceCreateInfo deviceInfo = {};
    deviceInfo.deviceExtensions = deviceExtensions;
    deviceInfo.validationLayers = validationLayers;
    deviceInfo.surface = surface;
    evkCreateDevice(physicalDevice, &deviceInfo, &device, &graphicsQueue, &presentQueue);

    EVkSwapchainCreateInfo swapchainInfo = {};
    swapchainInfo.physicalDevice = physicalDevice;
    swapchainInfo.surface = surface;
    swapchainInfo.window = window;
    swapchainInfo.numImages = MAX_FRAMES_IN_FLIGHT;
    evkCreateSwapchain(device, &swapchainInfo, &swapChain, &swapChainImages, &swapChainImageFormat, &swapChainExtent);
    
    EVkImageViewsCreateInfo imageViewsInfo = {};
    imageViewsInfo.images = swapChainImages;
    imageViewsInfo.swapChainImageFormat = swapChainImageFormat;
    evkCreateImageViews(device, &imageViewsInfo, &swapChainImageViews);

    EVkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.swapChainImageFormat = swapChainImageFormat;
    renderPassInfo.physicalDevice = physicalDevice;
    evkCreateRenderPass(device, &renderPassInfo, &renderPass);

    EVkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
    evkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, &descriptorSetLayout);

    EVkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.vertShaderFile = "shaders/vert.spv";
    pipelineInfo.fragShaderFile = "shaders/frag.spv";
    pipelineInfo.swapchainExtent = swapChainExtent;
    pipelineInfo.pDescriptorSetLayout = &descriptorSetLayout;
    pipelineInfo.renderPass = renderPass;
    evkCreateGraphicsPipeline(device, &pipelineInfo, &pipelineLayout, &graphicsPipeline);

    EVkDepthResourcesCreateInfo depthResourcesInfo = {};
    depthResourcesInfo.physicalDevice = physicalDevice;
    depthResourcesInfo.swapchainExtent = swapChainExtent;
    depthResourcesInfo.swapchainImageFormat = swapChainImageFormat;
    evkCreateDepthResources(device, &depthResourcesInfo, &depthImage, &depthImageView, &depthImageMemory);

    EVkFramebuffersCreateInfo framebuffersInfo = {};
    framebuffersInfo.swapchainExtent = swapChainExtent;
    framebuffersInfo.swapchainImageViews = swapChainImageViews;
    framebuffersInfo.renderPass = renderPass;
    framebuffersInfo.depthImageView = depthImageView;
    evkCreateFramebuffers(device, &framebuffersInfo, &swapChainFramebuffers);

    commandPools.resize(FLAGS_num_threads);
    for (auto &cp : commandPools)
    {
        EVkCommandPoolCreateInfo info = {};
        info.physicalDevice = physicalDevice;
        info.surface = surface;
        evkCreateCommandPool(device, &info, &cp);
    }

    EVkCommandPoolCreateInfo commandPoolInfo = {}; // TODO: Remove

    EVkIndexBufferCreateInfo indexBufferInfo = {};
    indexBufferInfo.commandPool = commandPools[0]; // TODO: Make multithreaded
    indexBufferInfo.physicalDevice = physicalDevice;
    indexBufferInfo.queue = graphicsQueue;
    indexBufferInfo.indices = indices;
    evkCreateIndexBuffer(device, &indexBufferInfo, &indexBuffer, &indexBufferMemory);

    EVkUniformBufferCreateInfo uniformBufferInfo = {};
    uniformBufferInfo.physicalDevice = physicalDevice;
    uniformBufferInfo.swapchainImages = swapChainImages;
    evkCreateUniformBuffers(device, &uniformBufferInfo, &uniformBuffers, &uniformBuffersMemory);

    EVkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.swapchainImages = swapChainImages;
    evkCreateDescriptorPool(device, &descriptorPoolInfo, &descriptorPool);

    EVkDescriptorSetCreateInfo descriptorSetInfo = {};
    descriptorSetInfo.descriptorPool = descriptorPool;
    descriptorSetInfo.descriptorSetLayout = descriptorSetLayout;
    descriptorSetInfo.swapchainImages = swapChainImages;
    descriptorSetInfo.uniformBuffers = uniformBuffers;
    evkCreateDescriptorSets(device, &descriptorSetInfo, &descriptorSets);

    EVkSyncObjectsCreateInfo syncObjectsInfo = {};
    syncObjectsInfo.maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
    syncObjectsInfo.swapchainSize = swapChainImages.size();
    evkCreateSyncObjects(device,
                         &syncObjectsInfo,
                         &imageAvailableSemaphores,
                         &renderFinishedSemaphores,
                         &inFlightFences,
                         &imagesInFlight);

    threadPool.setThreadCount(FLAGS_num_threads);

    EVkVertexBufferCreateInfo vUpdateInfo = {};
    vUpdateInfo.pVertices = &vertices;
    vUpdateInfo.physicalDevice = physicalDevice;
    vUpdateInfo.graphicsQueue = graphicsQueue;
    vUpdateInfo.vertexBuffer = vertexBuffer;
    vUpdateInfo.commandPools = commandPools;
    evkCreateVertexBuffer(device, &vUpdateInfo, &vertexBuffer, &vertexBufferMemory, threadPool);

    EVkCommandBuffersCreateInfo commandBuffersInfo = {};
    commandBuffersInfo.commandPool = commandPools[0]; // TODO: Is this needed?
    commandBuffersInfo.descriptorSets = descriptorSets;
    commandBuffersInfo.graphicsPipeline = graphicsPipeline;
    commandBuffersInfo.indexBuffer = indexBuffer;
    commandBuffersInfo.indices = indices;
    commandBuffersInfo.pipelineLayout = pipelineLayout;
    commandBuffersInfo.renderPass = renderPass;
    commandBuffersInfo.swapchainExtent = swapChainExtent;
    commandBuffersInfo.vertexBuffer = vertexBuffer;
    commandBuffersInfo.poolCreateInfo = commandPoolInfo;

    primaryCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    secondaryCommandBuffers.resize(FLAGS_num_threads);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        commandBuffersInfo.framebuffer=swapChainFramebuffers[i];
        evkCreateCommandBuffers(device,
            &commandBuffersInfo,
            &primaryCommandBuffers[i],
            &secondaryCommandBuffers,
            &commandPools,
            threadPool);
    }

    drawInfo.pInFlightFences = &inFlightFences;
    drawInfo.pImageAvailableSemaphores = &imageAvailableSemaphores;
    drawInfo.swapchain = swapChain;
    drawInfo.maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
    drawInfo.graphicsQueue = graphicsQueue;
    drawInfo.presentQueue = presentQueue;
    drawInfo.swapchainExtent = swapChainExtent;
    drawInfo.pUniformBufferMemory = &uniformBuffersMemory;
}

void EVulkan::mainLoop()
{
    int frameNum=0;
    bool timed=false;
    if (FLAGS_num_frames > 0) timed=true; 

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        bench.numVertices(vertices.size());
        bench.numThreads(FLAGS_num_threads);
        bench.numCubes(FLAGS_num_cubes);
        auto startTime = bench.start();
        evkDrawFrame(
            device,
            &drawInfo,
            &currentFrame, &imagesInFlight,
            &renderFinishedSemaphores,
            &primaryCommandBuffers[currentFrame]);
        bench.frameTime(startTime);
        bench.record();

        frameNum++;
        if (timed && frameNum >= FLAGS_num_frames) break;
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

    if (FLAGS_enable_validation)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}