#pragma once

#include "cube.h"
#include "evulkan.h"
#include <vector>

struct Grid
{
    Grid()=default;
    Grid(float _gridSize, float _cubeSize, size_t _num)
    {
        cubes = std::vector<Cube>();
        gridSize = _gridSize;
        float stepSize = _gridSize/_num;

        glm::vec3 color{1,1,1};
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

void createGrid(
    uint32_t numCubes,
    std::vector<evk::Vertex> &vertices,
    std::vector<uint32_t> &indices)
{
    constexpr size_t NUM_VERTS = 8;
    constexpr float GRID_SIZE = 2.0f;
    numCubes = sqrt(numCubes);
    float cubeSize = (GRID_SIZE/numCubes)*0.5;
    Grid grid = Grid(GRID_SIZE, cubeSize, numCubes);
    int i=0;
    evk::Vertex vertex;
    for (auto cube : grid.cubes)
    {
        std::vector<glm::vec3> verts = cube.vertices;
        std::vector<uint32_t> ind = cube.indices;
        for(size_t j = 0; j<verts.size(); ++j)
        {
            vertex.pos=verts[j];
            vertex.color={1,0,1};
            vertex.normal=-vertex.pos;
            vertices.push_back(vertex);
        }
        for(size_t j = 0; j<ind.size(); ++j)
        {
            indices.push_back(ind[j]+i*NUM_VERTS);
        }
        ++i;
    }
}