#ifndef EVK_FRAMEBUFFER
#define EVK_FRAMEBUFFER

#include "device.h"
#include "pass.h"
#include <vulkan/vulkan.h>

class Framebuffer
{
    public:
    Framebuffer()=default;
    Framebuffer(
        const Device &device,
        const Renderpass &renderpass,
        const Swapchain &swapchain
    );

    void destroy();

    std::vector<VkFramebuffer> m_framebuffers;

    private:
    VkDevice m_device;
};

#endif