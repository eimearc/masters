#ifndef EVK_VERTEX_H_
#define EVK_VERTEX_H_

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * A Vertex contains position, color, texture coordinate and normal information.
 **/
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

    bool operator==(const Vertex &other) const
    {
        if (pos!=other.pos) return false;
        if (color!=other.color) return false;
        if (texCoord!=other.texCoord) return false;
        if (normal!=other.normal) return false;
        return true;
    }

    bool operator!=(const Vertex &other) const
    {
        return !(*this==other);
    }
};

#endif