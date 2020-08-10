#ifndef EVK_PIPELINE_H_
#define EVK_PIPELINE_H_

#include "descriptor.h"
#include "device.h"
#include <gsl/gsl>
#include "shader.h"
#include "pass.h"
#include "util.h"
#include "vertexinput.h"
#include <vulkan/vulkan.h>

namespace evk {

/**
 * @class Pipeline
 * @brief A Pipeline is a sequence of operations which produces an image.
 * 
 * A Pipeline is constructed using all the state required to pass data from the
 * input assembly stage all the way to setting pixels. Each Pipeline has a
 * Subpass, Descriptor, VertexInput, Renderpass and set of Shaders.
 * 
 * A Pipeline sets up the fixed function stages of the program, along with
 * the custom information provided for setting up the programmable stages,
 * such as the vertex and fragment Shaders.
 * 
 * The Pipeline is then passed into the finalize method of the Device.
 * 
 * @example
 * Pipeline pipeline0(
 *  device, &subpass0, &descriptor0, vertexInput0, &renderpass, shaders0
 * );
 * Pipeline pipeline1(
 *  device, &subpass1, &descriptor1, vertexInput1, &renderpass, shaders1
 * );
 * std::vector<Pipeline*> pipelines = {&pipeline0, &pipeline1};
 * device.finalize(indexBuffer,vertexBuffer,pipelines);
 **/
class Pipeline
{
    public:
    Pipeline()=default;
    Pipeline(const Pipeline&)=delete;
    Pipeline& operator=(const Pipeline&)=delete;
    Pipeline(Pipeline&&) noexcept;
    Pipeline& operator=(Pipeline&&) noexcept;
    ~Pipeline() noexcept;

    /**
     * Creates a Pipeline with an attached descriptor.
     * @param[in] device the Device used to create the Pipeline.
     * @param[in] pSubpass the Subpass to use in this Pipeline.
     * @param[in] pDescriptor the Descriptor to use in this Pipeline.
     * @param[in] vertexInput the vertexInput for this Pipeline.
     * @param[in] renderpass the Renderpass for this Pipeline.
     * @param[in] shaders the set of Shaders used in this Pipeline.
     **/
    Pipeline(
        Device &device,
        Subpass *pSubpass,
        gsl::not_null<Descriptor*> pDescriptor,
        const VertexInput &vertexInput,
        Renderpass &renderpass,
        const std::vector<Shader*> &shaders
    );

    /**
     * Creates a Pipeline without an attached descriptor.
     * @param[in] device the Device used to create the Pipeline.
     * @param[in] pSubpass the Subpass to use in this Pipeline.
     * @param[in] vertexInput the vertexInput for this Pipeline.
     * @param[in] renderpass the Renderpass for this Pipeline.
     * @param[in] shaders the set of Shaders used in this Pipeline.
     **/
    Pipeline(
        Device &device,
        Subpass *pSubpass,
        const VertexInput &vertexInput,
        Renderpass &renderpass,
        const std::vector<Shader*> &shaders
    );

    bool operator==(const Pipeline&) const;

    private:
    void createSetLayout(
        const std::vector<VkDescriptorSetLayout> &setLayouts
    );
    void recreate();
    void reset();
    void setup();

    Descriptor* const descriptor() const { return m_descriptor; };
    VkPipelineLayout layout() const { return m_layout; };
    VkPipeline pipeline() const { return m_pipeline; };
    Renderpass* const renderpass() const { return m_renderpass; };

    Descriptor* m_descriptor=nullptr;
    Device *m_device=nullptr;
    VkPipelineLayout m_layout=VK_NULL_HANDLE;
    VkPipeline m_pipeline=VK_NULL_HANDLE;
    Renderpass *m_renderpass=nullptr;
    std::vector<Shader*> m_shaders;
    Subpass *m_subpass=nullptr;
    VertexInput m_vertexInput={};

    friend class Device;

    // Tests.
    FRIEND_TEST(PipelineTest,ctor);
};

} // end namespace evk

#endif