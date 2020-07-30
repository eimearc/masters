#ifndef EVK_PIPELINE_H_
#define EVK_PIPELINE_H_

#include "descriptor.h"
#include "device.h"
#include <gsl/gsl>
#include "shader.h"
#include "pass.h"
#include "util.h"
#include <vulkan/vulkan.h>

#include "vertexinput.h"

class Pipeline
{
    public:
    Pipeline()=default;
    Pipeline(const Pipeline&)=delete;
    Pipeline& operator=(const Pipeline&)=delete;
    Pipeline(Pipeline&&) noexcept;
    Pipeline& operator=(Pipeline&&) noexcept;
    ~Pipeline() noexcept;

    Pipeline(
        Device &device,
        Subpass *pSubpass, // TODO: For everything: reference or pointer?
        gsl::not_null<Descriptor*> pDescriptor,
        const VertexInput &vertexInput,
        Renderpass *pRenderpass,
        const std::vector<Shader*> &shaders
    );
    Pipeline(
        Device &device,
        Subpass *pSubpass,
        const VertexInput &vertexInput,
        Renderpass *pRenderpass,
        const std::vector<Shader*> &shaders
    );

    bool operator==(const Pipeline&) const;

    private:
    void createSetLayout(
        const std::vector<VkDescriptorSetLayout> &setLayouts
    );
    void reset();
    void setup(
        Device &device
    );

    Descriptor* const descriptor() const { return m_descriptor; };
    VkPipelineLayout layout() const { return m_layout; };
    VkPipeline pipeline() const { return m_pipeline; };
    Renderpass* const renderpass() const { return m_renderpass; };

    Descriptor* m_descriptor=nullptr;
    VkDevice m_device=VK_NULL_HANDLE;
    VkPipelineLayout m_layout=VK_NULL_HANDLE;
    VkPipeline m_pipeline=VK_NULL_HANDLE;
    Renderpass *m_renderpass=nullptr;
    std::vector<Shader*> m_shaders;
    Subpass *m_subpass=nullptr;
    VertexInput m_vertexInput;

    friend class Device;
};

#endif