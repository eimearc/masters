#include "shader.h"

Shader::Shader(const std::string &fileName, const ShaderStage &stage, const Device &device)
{   
    m_device=device.m_device;
    auto stageFlags = shaderStageFlags(stage);

    auto shaderCode = readFile(fileName);
    createShaderModule1(device.m_device, shaderCode, &m_module);

    m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_createInfo.stage = stageFlags;
    m_createInfo.module = m_module;
    m_createInfo.pName = "main";
    m_createInfo.pNext = nullptr;
    m_createInfo.flags = 0;
    m_createInfo.pSpecializationInfo = nullptr;
}

void Shader::createShaderModule1(
    VkDevice device,
    const std::vector<char>& code,
    VkShaderModule *pShaderModule)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, pShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module.");
    }
}

void Shader::destroy()
{
    vkDestroyShaderModule(m_device, m_module, nullptr);
}