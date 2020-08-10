#include "evulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

namespace evk {

TEST(OBJ, loadOBJ)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    evk::loadOBJ("tri.obj", vertices, indices);

    glm::vec3 color={0.1f,0.1f,0.1f};
    std::vector<Vertex> expectVertices = 
    {
        {{0.f,-0.5f,0.f},color,{0,1},{}}, {{-0.5f,0.5f,0.f},color,{0,0},{}},
        {{0.5f,0.5f,0.f},color,{1,1},{}} // Textures are flipped.
    };
    for (int i=0;i<vertices.size();++i)
    {
        EXPECT_FLOAT_EQ(vertices[i].pos.x,expectVertices[i].pos.x);
        EXPECT_FLOAT_EQ(vertices[i].pos.y,expectVertices[i].pos.y);
        EXPECT_FLOAT_EQ(vertices[i].pos.z,expectVertices[i].pos.z);

        EXPECT_FLOAT_EQ(vertices[i].pos.x,expectVertices[i].pos.x);
        EXPECT_FLOAT_EQ(vertices[i].pos.y,expectVertices[i].pos.y);
        EXPECT_FLOAT_EQ(vertices[i].pos.z,expectVertices[i].pos.z);

        EXPECT_FLOAT_EQ(vertices[i].normal.x,expectVertices[i].normal.x);
        EXPECT_FLOAT_EQ(vertices[i].normal.y,expectVertices[i].normal.y);
        EXPECT_FLOAT_EQ(vertices[i].normal.z,expectVertices[i].normal.z);

        EXPECT_FLOAT_EQ(vertices[i].texCoord.x,expectVertices[i].texCoord.x);
        EXPECT_FLOAT_EQ(vertices[i].texCoord.y,expectVertices[i].texCoord.y);
    }

    std::vector<uint32_t> expectIndices = {0,1,2};
    EXPECT_EQ(indices,expectIndices);
}

} // namespace evk