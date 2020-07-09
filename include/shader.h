#ifndef SHADER
#define SHADER

#include <vulkan/vulkan.h>
#include "util.h"
#include "device.h"

class Shader
{
    public:

    Shader()=default;
    Shader(const std::string &fileName, const ShaderStage &stage, const Device &device);
    void destroy();

    VkShaderModule m_module;
    VkPipelineShaderStageCreateInfo m_createInfo;

    private:
    void createShaderModule1(
        VkDevice device,
        const std::vector<char>& code,
        VkShaderModule *pShaderModule);

    VkDevice m_device;
};

#endif