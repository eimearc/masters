#ifndef EVK_SHADER_H_
#define EVK_SHADER_H_

#include "device.h"
#include "util.h"
#include <vulkan/vulkan.h>

class Shader
{
    public:
    // enum class Stage{VERTEX,FRAGMENT};

    Shader()=default;
    Shader(const Shader&)=delete;
    Shader& operator=(const Shader&)=delete;
    Shader(Shader&&) noexcept;
    Shader& operator=(Shader&&) noexcept;
    ~Shader() noexcept;

    bool operator==(const Shader&);

    Shader(
        const Device &device,
        const std::string &fileName,
        const ShaderStage &stage
    );

    VkPipelineShaderStageCreateInfo createInfo() const noexcept { return m_createInfo; };

    private:
    VkPipelineShaderStageCreateInfo m_createInfo;
    VkDevice m_device;
    VkShaderModule m_module=VK_NULL_HANDLE;
};

#endif