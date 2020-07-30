#ifndef EVK_ATTACHMENT_H_
#define EVK_ATTACHMENT_H_

#include "device.h"
#include <vulkan/vulkan.h>
#include "util.h"

class Attachment
{
    public:
    enum class Type{FRAMEBUFFER,COLOR,DEPTH};

    Attachment()=default;
    Attachment(const Attachment&)=delete;
    Attachment& operator=(const Attachment&)=delete;
    Attachment(Attachment&&) noexcept;
    Attachment& operator=(Attachment&&) noexcept;
    ~Attachment() noexcept;

    Attachment(
        const Device &device,
        uint32_t index,
        const Type &type
    );

    bool operator==(const Attachment&) const;

    private:
    void createFramebuffer();
    void setFramebufferAttachment();
    void setColorAttachment(
        const Device &device
    );
    void setDepthAttachment(
        const Device &device
    );
    void reset();

    VkClearValue clearValue() const { return m_clearValue; };
    VkAttachmentReference colorReference() const { return m_colorReference; };
    VkAttachmentReference depthReference() const { return m_depthReference; };
    VkAttachmentDescription description() const { return m_description; };
    uint32_t index() const { return m_index; };
    VkAttachmentReference inputReference() const { return m_inputReference; };
    VkImageView view() const { return m_imageView; };

    VkClearValue m_clearValue;
    VkAttachmentReference m_colorReference;
    VkAttachmentReference m_depthReference;
    VkAttachmentDescription m_description;
    VkDevice m_device;
    VkImage m_image=VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory=VK_NULL_HANDLE;
    VkImageView m_imageView=VK_NULL_HANDLE;
    uint32_t m_index;
    VkAttachmentReference m_inputReference;
    Type m_type;

    friend class Descriptor;
    friend class Device;
    friend class Renderpass;
    friend class Subpass;

    // Tests.
    friend class AttachmentTest_move_Test;
};

#endif