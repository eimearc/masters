#include "shader.h"

Shader::Shader(
    const Device &device,
    const std::string &fileName,
    const Stage &stage)
{   
    m_device=device.device();
    auto flags = stageFlags(stage);

    auto shaderCode = readFile(fileName);
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
    if (vkCreateShaderModule(device.device(), &createInfo, nullptr, &m_module) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module.");
    }

    m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_createInfo.stage = flags;
    m_createInfo.module = m_module;
    m_createInfo.pName = "main";
    m_createInfo.pNext = nullptr;
    m_createInfo.flags = 0;
    m_createInfo.pSpecializationInfo = nullptr;
}

Shader::Shader(Shader &&other) noexcept
{
    *this=std::move(other);
}

Shader& Shader::operator=(Shader &&other) noexcept
{
    if (*this==other) return *this;
    m_createInfo=other.m_createInfo;
    other.m_createInfo={};
    m_device=other.m_device;
    other.m_device=VK_NULL_HANDLE;
    m_module=other.m_module;
    other.m_module=VK_NULL_HANDLE;
    return *this;
}

bool Shader::operator==(const Shader &other) const
{
    if (m_createInfo.sType!=other.m_createInfo.sType) return false;
    if (m_createInfo.stage!=other.m_createInfo.stage) return false;
    if (m_createInfo.module!=other.m_createInfo.module) return false;
    if (*m_createInfo.pName!=*other.m_createInfo.pName) return false;
    if (m_createInfo.flags!=other.m_createInfo.flags) return false;
    return true;
}

bool Shader::operator!=(const Shader &other) const
{
    return !(*this==other);
}

Shader::~Shader() noexcept
{
    if (m_module!=VK_NULL_HANDLE)
        vkDestroyShaderModule(m_device, m_module, nullptr);
}

VkShaderStageFlagBits Shader::stageFlags(const Stage &shaderStage)
{
    switch (shaderStage)
    {
    case Stage::VERTEX:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case Stage::FRAGMENT:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    }
}

std::vector<char> Shader::readFile(const std::string& filename) const
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file.");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}