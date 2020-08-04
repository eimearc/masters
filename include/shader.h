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
    enum class Stage{VERTEX,FRAGMENT}; // TODO: Add support for Geometry shader.

    Shader()=default;
    Shader(const Shader&)=delete;
    Shader& operator=(const Shader&)=delete;
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

} // namespace evk

#endif