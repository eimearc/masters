#include "evulkan.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace evk {

void evk::loadOBJ(
    const std::string &fileName,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(
        &attrib, &shapes, &materials, &warn, &err, fileName.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex{};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            vertex.color = {0.1,0.1,0.1};

            vertices.push_back(vertex);
            indices.push_back(indices.size());
        }
    }
}

} // namespace evk