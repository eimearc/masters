#include "evulkan.h"

void EVulkan::initVulkan()
{
    evkCreateWindow(EVkCreateWindow{}, window);

    evk::InstanceCreateInfo instanceCreateInfo{
        validationLayers,
        window,
        deviceExtensions
    };
    evk::Instance evkInstance{&instanceCreateInfo};
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

    evk::SwapChainCreateInfo swapChainCreateInfo{
        static_cast<uint8_t>(MAX_FRAMES_IN_FLIGHT)
    };
    evkInstance.createSwapChain(&swapChainCreateInfo);
    swapChain=evkInstance.m_swapChain;
    swapChainImages=evkInstance.m_swapChainImages;
    swapChainImageFormat=evkInstance.m_swapChainImageFormat;
    swapChainExtent=evkInstance.m_swapChainExtent;
    swapChainImageViews=evkInstance.m_swapChainImageViews;

    evkInstance.createRenderPass();
    renderPass=evkInstance.m_renderPass;

    // ----------------
    // Below are linked
    EVkUniformBufferCreateInfo uniformBufferInfo = {};
    uniformBufferInfo.physicalDevice = physicalDevice;
    uniformBufferInfo.swapchainImages = swapChainImages;
    evkCreateUniformBuffers(device, &uniformBufferInfo, &uniformBuffers, &uniformBuffersMemory);
    evkInstance.m_uniformBuffers=uniformBuffers;
    evkInstance.m_uniformBuffersMemory=uniformBuffersMemory;

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

    EVkFramebuffersCreateInfo framebuffersInfo = {};
    framebuffersInfo.swapchainExtent = swapChainExtent;
    framebuffersInfo.swapchainImageViews = swapChainImageViews;
    framebuffersInfo.renderPass = renderPass;
    framebuffersInfo.depthImageView = depthImageView;
    evkCreateFramebuffers(device, &framebuffersInfo, &swapChainFramebuffers);

    EVkCommandPoolCreateInfo commandPoolInfo = {}; // TODO: Remove

    EVkIndexBufferCreateInfo indexBufferInfo = {};
    indexBufferInfo.commandPool = commandPools[0]; // TODO: Make multithreaded
    indexBufferInfo.physicalDevice = physicalDevice;
    indexBufferInfo.queue = graphicsQueue;
    indexBufferInfo.indices = indices;
    evkCreateIndexBuffer(device, &indexBufferInfo, &indexBuffer, &indexBufferMemory);

    threadPool.setThreadCount(FLAGS_num_threads);

    EVkVertexBufferCreateInfo vUpdateInfo = {};
    vUpdateInfo.pVertices = &vertices;
    vUpdateInfo.physicalDevice = physicalDevice;
    vUpdateInfo.graphicsQueue = graphicsQueue;
    vUpdateInfo.vertexBuffer = vertexBuffer;
    vUpdateInfo.commandPools = commandPools;
    evkCreateVertexBuffer(device, &vUpdateInfo, &vertexBuffer, &vertexBufferMemory, threadPool);

    EVkSyncObjectsCreateInfo syncObjectsInfo = {};
    syncObjectsInfo.maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
    syncObjectsInfo.swapchainSize = swapChainImages.size();
    evkCreateSyncObjects(device,
                         &syncObjectsInfo,
                         &imageAvailableSemaphores,
                         &renderFinishedSemaphores,
                         &inFlightFences,
                         &imagesInFlight);

    EVkCommandBuffersCreateInfo commandBuffersInfo = {};
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

    if (validationLayers.size() > 0)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}