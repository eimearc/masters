#ifndef EVK_SHADER_H_
#define EVK_SHADER_H_

#include "device.h"
#include "util.h"
#include <vulkan/vulkan.h>

namespace evk {

/**
 * @class Shader
 * @brief A Shader processes vertices and fragments to produce images.
 * 
 * A Shader is a program which is written by a user, compiled, and produced as
 * SPIR-V bytecode. There are two types of supported shaders, a VERTEX shader
 * and a FRAGMENT shader. These are passed to the program through their
 * filenames.
 * 
 * One of both the VERTEX and FRAGMENT shader must be provided to a Pipeline,
 * where it is bound and executed.
 * 
 * @example
 * Shader vertexShader(device, "shader_vert.spv", Shader::Stage::VERTEX);
 * Shader fragmentShader(device, "shader_frag.spv", Shader::Stage::FRAGMENT);
 * std::vector<Shader*> shaders = {&vertexShader,&fragmentShader};
 * 
 * Pipeline pipeline(
 *  device, &subpass, &descriptor, vertexInput, &renderpass, shaders
 * );
 **/
class Shader
{
    public:
    /**
     * The stage at which the Shader runs.
     * VERTEX: A vertex Shader.
     * FRAGMENT: A fragments Shader.
     **/
    enum class Stage{VERTEX,FRAGMENT};

    Shader()=default;
    Shader(const Shader&)=delete; // Class Shader is non-copyable.
    Shader& operator=(const Shader&)=delete; // Class Shader is non-copyable.
    Shader(Shader&&) noexcept;
    Shader& operator=(Shader&&) noexcept;
    ~Shader() noexcept;

    /**
     * Constructs a new Shader.
     * @param[in] device the Device to use for creating the Shader.
     * @param[in] fileName the file where the Shader code is.
     * @param[in] stage the stage when this Shader will be executed.
     **/
    Shader(
        const Device &device,
        const std::string &fileName,
        const Stage &stage
    );

    bool operator==(const Shader&) const noexcept;
    bool operator!=(const Shader&) const noexcept;

    void reset() noexcept;

    private:
    static VkShaderStageFlagBits stageFlags(const Stage &stage) noexcept;

    VkPipelineShaderStageCreateInfo createInfo() const noexcept
    {
        return m_createInfo;
    };
    std::vector<char> readFile(const std::string& filename) const;

    VkPipelineShaderStageCreateInfo m_createInfo;
    VkDevice m_device;
    VkShaderModule m_module=VK_NULL_HANDLE;

    friend class Descriptor;
    friend class Pipeline;

    // Tests.
    FRIEND_TEST(ShaderTest,ctor);
};

} // namespace evk

#endif