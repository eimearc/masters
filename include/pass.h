#ifndef EVK_PASS_H_
#define EVK_PASS_H_

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

    bool operator==(const Subpass&) const;

    bool hasDepthAttachment() const { return !m_depthAttachments.empty(); };
    std::vector<VkSubpassDependency> dependencies() const { return m_dependencies; };
    VkSubpassDescription description() const { return m_description; };
    uint32_t index() const { return m_index; };

    private:
    void addDependency(uint32_t srcSubpass, uint32_t dstSubpass);

    std::vector<Attachment*> m_colorAttachments;
    std::vector<VkAttachmentReference> m_colorReferences;
    std::vector<VkSubpassDependency> m_dependencies;
    std::vector<Attachment*> m_depthAttachments;
    std::vector<VkAttachmentReference> m_depthReferences;
    VkSubpassDescription m_description;
    uint32_t m_index;
    std::vector<Attachment*> m_inputAttachments;
    std::vector<VkAttachmentReference> m_inputReferences;

    friend class Pipeline;
};

class Renderpass
{
    public:
    Renderpass()=default;
    Renderpass(const Renderpass&)=delete;
    Renderpass& operator=(const Renderpass&)=delete;
    Renderpass(Renderpass&&) noexcept;
    Renderpass& operator=(Renderpass&&) noexcept;
    ~Renderpass() noexcept;

    Renderpass(
        const Device &device,
        const std::vector<Attachment*> &attachments,
        std::vector<Subpass*> &subpasses
    );

    bool operator==(const Renderpass&) const;

    private:
    const std::vector<Attachment*>& attachments() const { return m_attachments; };
    std::vector<VkClearValue> clearValues() const { return m_clearValues; };
    VkRenderPass renderpass() const { return m_renderPass; };
    std::vector<Subpass*> subpasses() const { return m_subpasses; };

    std::vector<Attachment*> m_attachments;
    std::vector<VkClearValue> m_clearValues;
    VkDevice m_device;
    VkRenderPass m_renderPass=VK_NULL_HANDLE;
    std::vector<Subpass*> m_subpasses;

    friend class Device;
    friend class Framebuffer;
    friend class Pipeline;
};

#endif