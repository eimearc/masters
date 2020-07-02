#ifndef SHADER
#define SHADER

#include <vulkan/vulkan.h>
#include "evulkan_util.h"
#include "device.h"

class Shader
{
    public:
    enum Stage{Vertex, Fragment};

    Shader()=default;
    Shader(const std::string &fileName, const Stage &stage, const Device &device);
    void destroy();

    VkShaderModule m_module;
    VkPipelineShaderStageCreateInfo m_createInfo;

    private:
    void createShaderModule1(
        VkDevice device,
        const std::vector<char>& code,
        VkShaderModule *pShaderModule);
};

#endif