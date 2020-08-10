#ifndef EVK_PASS_H_
#define EVK_PASS_H_

#include "attachment.h"
#include "util.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace evk {
/**
 * @class Subpass
 * @brief A Subpass is a sequence of rendering operations.
 * 
 * A Subpass is a sequence of rendering operations which may preceed or follow
 * another Subpass. One or more Subpasses make up a Renderpass. A Subpass
 * contains a series of Attachments - color Attachments, depth
 * Attachments and input Attachments.
 * 
 * A Subpass also has an index, which specifies the order in which subsequent
 * Subpasses are executed. Each Subpass must have a unique index, and the
 * indices must begin from 0 and increase with no gaps.
 * 
 * A Subpass may also depend on the completion of other Subpasses. If a
 * Subpass depends on another Subpass, it waits for the fragment Shader
 * stage of that previous Subpass to complete before starting. This 
 * allows multipass rendering.
 **/
class Subpass
{
    public:
    typedef uint32_t Dependency;

    Subpass()=default;
    Subpass(
        const uint32_t index,
        const std::vector<Dependency> &dependencies,
        const std::vector<Attachment*> &colorAttachments,
        const std::vector<Attachment*> &depthAttachments,
        const std::vector<Attachment*> &inputAttachments
    );

    bool operator==(const Subpass&) const;

    private:
    void addDependency(Dependency dependency);
    static bool referenceEqual(
        const VkAttachmentReference &a,
        const VkAttachmentReference &b
    );

    std::vector<VkSubpassDependency> dependencies() const { return m_dependencies; };
    VkSubpassDescription description() const { return m_description; };
    bool hasDepthAttachment() const { return !m_depthAttachments.empty(); };
    uint32_t index() const { return m_index; };

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
    friend class Renderpass;

    // Tests.
    FRIEND_TEST(PassTest,ctor);
};

/**
 * @class Renderpass
 * @brief A Renderpass is a collection of Subpasses.
 **/
class Renderpass
{
    public:
    Renderpass()=default;
    Renderpass(const Renderpass&)=delete;
    Renderpass& operator=(const Renderpass&)=delete;
    Renderpass(Renderpass&&) noexcept;
    Renderpass& operator=(Renderpass&&) noexcept;
    ~Renderpass() noexcept;

    /**
     * Constructs a Renderpass.
     * 
     * @param[in] device the Device to use when creating the Renderpass.
     * @param[in] subpasses the Subpasses that make up this Renderpass.
     **/
    Renderpass(
        const Device &device,
        std::vector<Subpass*> &subpasses
    );

    bool operator==(const Renderpass&) const;

    private:
    struct AttachmentInfo
    {
        std::vector<Attachment*> attachments;
        std::vector<VkAttachmentDescription> descriptions;
        std::vector<VkClearValue> clearValues;
    };
    static AttachmentInfo attachmentInfo(
        const std::vector<Subpass*> &subpasses
    );
    static void setAttachmentInfo(
        AttachmentInfo &info, Attachment *pAttachment
    );

    const std::vector<Attachment*>& attachments() const { return m_attachments; };
    std::vector<VkClearValue> clearValues() const { return m_clearValues; };
    VkRenderPass renderpass() const { return m_renderPass; };
    void reset();
    std::vector<Subpass*> subpasses() const { return m_subpasses; };

    std::vector<Attachment*> m_attachments;
    std::vector<VkClearValue> m_clearValues;
    VkDevice m_device=VK_NULL_HANDLE;
    VkRenderPass m_renderPass=VK_NULL_HANDLE;
    std::vector<Subpass*> m_subpasses;

    friend class Device;
    friend class Framebuffer;
    friend class Pipeline;

    // Tests.
    FRIEND_TEST(PassTest,constructDescriptions);
    FRIEND_TEST(PassTest,ctor);
};

} // namespace evk

#endif