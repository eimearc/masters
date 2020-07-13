#ifndef EVK_CORE
#define EVK_CORE

#include <array>
#include <fstream>
#include <GLFW/glfw3.h>
#include <map>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

#include "attachment.h"
#include "buffer.h"
#include "command.h"
#include "descriptor.h"
#include "device.h"
#include "draw.h"
#include "framebuffer.h"
#include "pass.h"
#include "pipeline.h"
#include "shader.h"
#include "swapchain.h"
#include "texture.h"
#include "threadpool.h"
#include "sync.h"

#define GLFW_INCLUDE_VULKAN

#endif