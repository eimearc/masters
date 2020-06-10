#pragma once

#include <vector>
#include "cube.h"

struct Grid
{
    Grid()=default;
    Grid(float _gridSize, float _cubeSize, size_t _num)
    {
        cubes = std::vector<Cube>();
        gridSize = _gridSize;
        float stepSize = _gridSize/_num;

        glm::vec3 color{1,0,0};
        glm::vec3 center{0,0,0};
        float left = (center.x-_gridSize/2.0f)+0.5f*stepSize;
        float top = (center.y+_gridSize/2.0f)-0.5f*stepSize;
        for (int i=0; i<_num; ++i)
        {
            for (int j=0; j<_num; ++j)
            {
                center = {(left+j*stepSize),(top-i*stepSize),0.0f};
                cubes.push_back(Cube(center, color, _cubeSize));
            }
        }
    }
    ~Grid()noexcept=default;

    std::vector<Cube> cubes;
    float gridSize;
    size_t num;
};