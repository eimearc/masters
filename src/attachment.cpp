#include "attachment.h"

namespace evk {

Attachment::Attachment(Attachment &&other) noexcept
{
    *this=std::move(other);
}

Attachment& Attachment::operator=(Attachment &&other) noexcept
{
    if (*this==other) return *this;
    m_clearValue=other.m_clearValue;
    m_colorReference=other.m_colorReference;
    m_depthReference=other.m_depthReference;
    m_description=other.m_description;
    m_device=other.m_device;
    m_image=other.m_image;
    m_imageMemory=other.m_imageMemory;
    m_imageView=other.m_imageView;
    m_index=other.m_index;
    m_inputReference=other.m_inputReference;
    m_type=other.m_type;
    other.reset();
    return *this;
}

void Attachment::reset()
{
    m_clearValue={};
    m_colorReference={};
    m_depthReference={};
    m_description={};
    m_device=VK_NULL_HANDLE;
    m_image=VK_NULL_HANDLE;
    m_imageMemory=VK_NULL_HANDLE;
    m_imageView=VK_NULL_HANDLE;
    m_index=0;
    m_inputReference={};
}

Attachment::Attachment(
    const Device &device,
    uint32_t index,
    const Type &type)
{
    m_device = device.device();
    m_index = index;
    m_type = type;

    m_inputReference = {index, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    m_colorReference = {index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    m_depthReference = {index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    switch(type)
    {
        case Type::FRAMEBUFFER:
            setFramebufferAttachment();
            break;
        case Type::COLOR:
            setColorAttachment(device);
            break;
        case Type::DEPTH:
            setDepthAttachment(device);
            break;
    }
}

bool Attachment::operator==(const Attachment &other) const
{
    bool result=true;
    result &= (m_description.format==other.m_description.format);
    result &= (m_image==other.m_image);
    result &= (m_imageMemory==other.m_imageMemory);
    result &= (m_imageView==other.m_imageView);
    result &= (m_index==other.m_index);
    result &= (m_type==other.m_type);
    return result;
}

void Attachment::setFramebufferAttachment()
{
    m_description.flags = 0;
    m_description.format = VK_FORMAT_B8G8R8A8_SRGB;
    m_description.samples = VK_SAMPLE_COUNT_1_BIT;
    m_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    m_clearValue.color = {0.0f,0.0f,0.0f,1.0f};
}

void Attachment::setColorAttachment(const Device &device)
{
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    m_description.flags = 0;
    m_description.format = format;
    m_description.samples = VK_SAMPLE_COUNT_1_BIT;
    m_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    internal::createImage(
        device.device(), device.physicalDevice(),
        device.extent(), format, tiling, usage, properties,
        &m_image, &m_imageMemory);

    internal::createImageView(
        device.device(), m_image, format,
        aspectMask, &m_imageView);

    m_clearValue.color = {0.0f,0.0f,0.0f,1.0f};
}

void Attachment::setDepthAttachment(const Device &device)
{
    m_description.flags = 0;
    m_description.format = device.depthFormat();
    m_description.samples = VK_SAMPLE_COUNT_1_BIT;
    m_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    m_format = device.depthFormat();
    m_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    m_aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    internal::createImage(
        device.device(), device.physicalDevice(),
        device.extent(), m_format, m_tiling, m_usage, m_properties,
        &m_image, &m_imageMemory);

    internal::createImageView(
        device.device(), m_image, m_format,
        m_aspectMask, &m_imageView);

    m_clearValue.depthStencil = {1.0f,1};
}

void Attachment::recreate(Device &device)
{
    switch (m_type)
    {
    case Type::FRAMEBUFFER:
        return;

    default:
        if (m_imageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_imageView, nullptr);
        if (m_image != VK_NULL_HANDLE) vkDestroyImage(m_device, m_image, nullptr); 
        internal::createImage(
            device.device(), device.physicalDevice(),
            device.extent(), m_format, m_tiling, m_usage, m_properties,
            &m_image, &m_imageMemory
        );
        internal::createImageView(
            device.device(), m_image, m_format,
            m_aspectMask, &m_imageView
        );
        break;
    }
}

Attachment::~Attachment() noexcept
{
    if (m_imageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_imageView, nullptr);
    if (m_image != VK_NULL_HANDLE) vkDestroyImage(m_device, m_image, nullptr);
    if (m_imageMemory != VK_NULL_HANDLE) vkFreeMemory(m_device, m_imageMemory, nullptr);
}

} // namespace evk