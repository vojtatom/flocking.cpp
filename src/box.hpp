#pragma once
#include <cstddef>
#include <glm/glm.hpp>

/**
 * @brief Representation of bounding box
 * 
 * Bounding box defined by two diagonally opposite vertices
 */
struct Box{
    /**
     * @brief Vertex with the minimal cooradinates
     */
    glm::vec3 low;
    /**
     * @brief Vertex with the maximal cooradinates
     */
    glm::vec3 high;
};
