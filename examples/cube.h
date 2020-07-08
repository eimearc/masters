#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Cube
{
    Cube()=default;
    Cube(glm::vec3 _center, glm::vec3 _color, float _size)
    {
        float half = _size/2.0f;
        float pos_x = _center.x + half;
        float neg_x = _center.x - half;
        float pos_y = _center.y + half;
        float neg_y = _center.y - half;
        float pos_z = _center.z + half;
        float neg_z = _center.z - half;

        vertices = {
            // Top
            {neg_x, neg_y, pos_z},  // bottom left
            {pos_x, neg_y, pos_z},   // bottom right
            {pos_x, pos_y, pos_z},   // top right
            {neg_x, pos_y, pos_z},  // top left
            // Bottom
            {neg_x, neg_y, neg_z}, // bottom left
            {pos_x, neg_y, neg_z},  // bottom right
            {pos_x, pos_y, neg_z},   // top right
            {neg_x, pos_y, neg_z},
        };
        indices = {
            0, 1, 2, 2, 3, 0, // top
            0, 4, 5, 5, 1, 0, // side 0
            1, 5, 6, 6, 2, 1, // side 1
            2, 6, 7, 7, 3, 2, // side 2
            3, 7, 4, 4, 0, 3,  // side 3
            4, 6, 5, 6, 4, 7, // bottom
        };
        color = _color;
        center = _center;
    }
    ~Cube()noexcept=default;

    glm::vec3 center;
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 color={0,1,0};
};