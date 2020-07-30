#ifndef EVK_VERTEX_H_
#define EVK_VERTEX_H_

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

    bool operator==(const Vertex &other) const
    {
        bool result = true;
        result &= (pos==other.pos);
        result &= (color==other.color);
        result &= (texCoord==other.texCoord);
        result &= (normal==other.normal);
        return result;
    }
};

#endif