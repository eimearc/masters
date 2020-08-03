#ifndef EVK_SHADER_H_
#define EVK_SHADER_H_

#include "device.h"
#include "util.h"
#include <vulkan/vulkan.h>

class Shader
{
    public:
    enum class Stage{VERTEX,FRAGMENT}; // TODO: Add support for Geometry shader.

    Shader()=default;
    Shader(const Shader&)=delete;
    Shader& operator=(const Shader&)=delete;
    Shader(Shader&&) noexcept;
    Shader& operator=(Shader&&) noexcept;
    ~Shader() noexcept;

    Shader(
        const Device &device,
        const std::string &fileName,
        const Stage &stage
    );

    bool operator==(const Shader&) const;
    bool operator!=(const Shader&) const;

    private:
    VkPipelineShaderStageCreateInfo createInfo() const noexcept { return m_createInfo; };
    static VkShaderStageFlagBits stageFlags(const Stage &stage);
    std::vector<char> readFile(const std::string& filename) const;

    VkPipelineShaderStageCreateInfo m_createInfo;
    VkDevice m_device;
    VkShaderModule m_module=VK_NULL_HANDLE;

    friend class Descriptor;
    friend class Pipeline;

    // Tests.
    friend class ShaderTest_ctor_Test;
};

#endif