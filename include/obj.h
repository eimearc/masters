#ifndef EVK_OBJ_H_
#define EVK_OBJ_H_

#include <string>
#include <vector>
#include "vertex.h"

namespace evk {

/**
 * Loads and OBJ file into a vector of vertices and indices.
 * @param[in] fileName the file where the OBJ is contained.
 * @param[out] vertices the vertices of the OBJ.
 * @param[out] indices the indices of the OBJ.
 **/
void loadOBJ(
    const std::string &fileName,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices
);

} // namespace evk

#endif