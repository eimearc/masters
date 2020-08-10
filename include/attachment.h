#ifndef EVK_ATTACHMENT_H_
#define EVK_ATTACHMENT_H_

#include "device.h"
#include <vulkan/vulkan.h>
#include "util.h"

namespace evk {

/**
 * @class Attachment
 * @brief An Attachment is a resource bound to a Shader.
 * 
 * Attachments are used within Subpasses. They are bound to Shaders and can
 * be written to and read from.
 * 
 * The Framebuffer Attachment is reserved at index 0. It is used to display
 * images on the screen. The fragment Shader in the final Subpass writes to
 * this Attachment.
 * 
 * @example
 * Attachment framebufferAttachment(device, 0, Attachment::Type::FRAMEBUFFER);
 * Attachment colorAttachment(device, 1, Attachment::Type::COLOR);
 * Attachment depthAttachment(device, 2, Attachment::Type::DEPTH);
 **/
class Attachment
{
    public:
    /**
     * The Type of an Attachment.
     * FRAMEBUFFER: an Attachment used for blitting images to the screen.
     * COLOR: an Attachment used for color images.
     * DEPTH: an Attachmend used for depth images.
     **/
    enum class Type{FRAMEBUFFER,COLOR,DEPTH};

    Attachment()=default;
    Attachment(const Attachment&)=delete; // Class Attachment is not copyable.
    Attachment& operator=(const Attachment&)=delete; // Class Attachment is not copyable.
    Attachment(Attachment&&) noexcept;
    Attachment& operator=(Attachment&&) noexcept;
    ~Attachment() noexcept;

    /**
     * Creates an Attachment.
     * @param[in] device the device used to create the Attachment.
     * @param[in] index the index of the Attachment.
     *  Index 0 is reserved for the FRAMEBUFFER Attachment.
     * @param[in] type the Type of the Attachment.
     * @returns a new Attachment.
     **/
    Attachment(
        const Device &device,
        uint32_t index,
        const Type &type
    );

    bool operator==(const Attachment&) const noexcept;
    bool operator!=(const Attachment&) const noexcept;

    private:
    void createFramebuffer();
    void setFramebufferAttachment();
    void setColorAttachment(
        const Device &device
    );
    void setDepthAttachment(
        const Device &device
    );
    void recreate(Device &device);
    void reset();

    VkClearValue clearValue() const { return m_clearValue; };
    VkAttachmentReference colorReference() const { return m_colorReference; };
    VkAttachmentReference depthReference() const { return m_depthReference; };
    VkAttachmentDescription description() const { return m_description; };
    uint32_t index() const { return m_index; };
    VkAttachmentReference inputReference() const { return m_inputReference; };
    VkImageView view() const { return m_imageView; };

    VkImageAspectFlags m_aspectMask;
    VkClearValue m_clearValue={};
    VkAttachmentReference m_colorReference;
    VkAttachmentReference m_depthReference;
    VkAttachmentDescription m_description;
    VkDevice m_device;
    VkFormat m_format;
    VkImage m_image=VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory=VK_NULL_HANDLE;
    VkImageView m_imageView=VK_NULL_HANDLE;
    uint32_t m_index;
    VkAttachmentReference m_inputReference;
    VkMemoryPropertyFlags m_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    Type m_type;
    VkImageTiling m_tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags m_usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    friend class Descriptor;
    friend class Device;
    friend class Pipeline;
    friend class Renderpass;
    friend class Subpass;

    // Tests.
    FRIEND_TEST(AttachmentTest,ctor);
    FRIEND_TEST(AttachmentTest,move);
    FRIEND_TEST(PassTest,constructDescriptions);
};

} // namespace evk

#endif