#include "util.h"

std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file.");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

void update(std::vector<Vertex> &vertices, const Grid &grid, size_t startOffset, size_t numVerts)
{
    glm::mat3 rotate = glm::rotate(glm::mat4(1.0f), 0.01f * glm::radians(90.0f), glm::vec3(1.0f,0.0f,0.0f));
    for (size_t i = startOffset; i < startOffset+numVerts; ++i)
    {
        int j = floor(i/8);
        glm::vec3 center = grid.cubes[floor(i/8)].center;
        glm::vec3 tmp = vertices[i].pos;
        tmp -= center;
        tmp = rotate*tmp;
        tmp += center;
        vertices[i].pos = tmp;
    }
}

UniformBufferObject getUBO(const uint32_t &_width, const uint32_t &_height)
{
    UniformBufferObject ubo = {};
    ubo.model=glm::mat4(1.0f);
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), _width/(float)_height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    return ubo;
}