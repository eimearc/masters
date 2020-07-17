#ifndef EVK_PASS
#define EVK_PASS

#include "attachment.h"
#include "util.h"
#include <vector>
#include <vulkan/vulkan.h>

class Subpass
{
    public:
    Subpass(
        const uint32_t index,
        const std::vector<evk::SubpassDependency> &dependencies,
        const std::vector<Attachment*> &colorAttachments,
        const std::vector<Attachment*> &depthAttachments,
        const std::vector<Attachment*> &inputAttachments
    );

    uint32_t m_index;
    VkSubpassDescription m_description;
    std::vector<VkSubpassDependency> m_dependencies;
    std::vector<VkAttachmentReference> m_colorReferences;
    std::vector<VkAttachmentReference> m_depthReferences;
    std::vector<VkAttachmentReference> m_inputReferences;

    private:
    void addDependency(uint32_t srcSubpass, uint32_t dstSubpass);
    std::vector<Attachment*> m_colorAttachments;
    std::vector<Attachment*> m_depthAttachments;
    std::vector<Attachment*> m_inputAttachments;
};

class Renderpass
{
    public:
    Renderpass()=default;
    Renderpass(const Renderpass&)=delete;
    Renderpass& operator=(const Renderpass&)=delete;
    Renderpass(Renderpass&&)=delete;
    Renderpass& operator=(Renderpass&&)=delete;
    ~Renderpass() noexcept;

    Renderpass(
        const Device &device,
        const std::vector<Attachment*> &attachments,
        std::vector<Subpass*> &subpasses
    );

    const std::vector<Attachment*>& attachments() const { return m_attachments; };
    std::vector<VkClearValue> clearValues() const { return m_clearValues; };
    VkRenderPass renderpass() const { return m_renderPass; };
    std::vector<Subpass*> subpasses() const { return m_subpasses; };

    private:
    VkDevice m_device;
    VkRenderPass m_renderPass;
    std::vector<Subpass*> m_subpasses;
    std::vector<VkClearValue> m_clearValues;
    std::vector<Attachment*> m_attachments;
};

#endif