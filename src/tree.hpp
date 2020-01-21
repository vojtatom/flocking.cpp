#pragma once
#include <cstddef>
#include <glm/glm.hpp>

#include "box.hpp"
#include "boid.hpp"

#define CHILDPERNODE 4
#define SUBTREES 8


class OctalTreeContainer;

/**
 * @brief Octree node
 * 
 * Contains 8 subtrees and 4 nodes.
 */
struct OctalTree {
    /**
     * @brief Construct a new Octree
     */
    OctalTree();

    /**
     * @brief Resets the node, sets contents to empty
     */
    void reset();

    /**
     * @brief Sets the box of the node
     * 
     * @param box Bounding box to be assigned
     */
    void set(const Box & box);

    /**
     * @brief Setup the box of the node
     * 
     * Calculates the bounding box of the node based on the
     * index of node in the parent node.
     * 
     * @param box Bounding box of the parent node
     * @param treeIdx Index of this node in the parent node array
     */
    void set(const Box & box,  unsigned char treeIdx);

    /**
     * @brief Test function whether the node is covered by the 
     * search range
     * 
     * @param point Center of the spehre 
     * @param rangeSqr Squared search range
     * @return true Node is covered by the search range
     * @return false Node is not covered by the sreach range
     */
    bool covered(glm::vec3 point, float rangeSqr);

    /** @brief Bounding box of the node */
    Box bbox; //24 Bytes
    /** @brief Array of indicies correspongin to child nodes */
    int32_t subtree[SUBTREES]; //32 Bytes
    /** @brief Array of indicies of the contained boids */
    int32_t boidIdx[CHILDPERNODE]; //16 Bytes
    /** @brief Number of boids in the node */
    int32_t filled; //4 Bytes
}; //80 Bytes in total, could be improved to 64 Bytes


#define MAXFOUND 1024

/**
 * @brief Octree container 
 * 
 * All nodes are lineary stored in the array of nodes.
 */
class OctalTreeContainer {
public:
    /**
     * @brief Construct a new Octal Tree Container object
     * 
     * @param bbox Global bounding box
     * @param boids Boid container
     */
    OctalTreeContainer(const Box & bbox, const BoidContainer * boids);
    
    /**
     * @brief Destroy the Octal Tree Container object
     */
    ~OctalTreeContainer();

    /**
     * @brief Insert boid into the tree
     * @param boid Boid to be inserted
     */
    void insert(const Boid & boid);

    /**
     * @brief Range search
     * 
     * @param point Search point
     * @param range Maximal distance from the search point to be included into search results
     */
    void findRange(glm::vec3 point, float range);

    /**
     * @brief Reset the tree, set all to empty
     */
    void reset();

    /**
     * @brief Get the tightly packed array of tree nodes
     * @param count Reference filled with number of tree nodes
     * @return OctalTree* 
     */
    OctalTree * getTrees(int32_t & count) const;

    /**
     * @brief Get all boids found in last search query
     * 
     * @param count Reference filled with number of found boids
     * @return const int32_t* Pointer to array of found boid indicies
     */
    const int32_t * getBoidsInRange(uint32_t & count) const;

    /**
     * @brief Get returns coordinate of the lowest bounding box vertex
     * @return const glm::vec3& low BB vertex
     */
    const glm::vec3 & getBoxLow() const;

    /**
     * @brief Get returns coordinate of the highest bounding box vertex
     * @return const glm::vec3& high BB vertex
     */
    const glm::vec3 & getBoxHigh() const;

    /**
     * @brief Get the dimensions of the bounding box
     * @return const glm::vec3 BB dimensions
     */
    const glm::vec3 getBoxDiagonal() const;

    /**
     * @brief Prints statistics for tree traversal
     */
    void printStats();

private:
    /**
     * @brief Recursive method for inserting boids into the tree
     * 
     * @param boid Boid to be inserted
     * @param treeIdx Index of the tree node reached during the insertion
     */
    void insert(const Boid & boid, int32_t treeIdx);

    /**
     * @brief Recursive method for range search
     * 
     * @param point Search point
     * @param rangeSqr Maximal distance from the search point to be included into search results, squared
     * @param treeIdx Index of the tree node reached during the traversal
     */
    void findRange(glm::vec3 point, float rangeSqr, int32_t treeIdx);

    /**
     * @brief Yields new tree node
     *
     * @return int32_t index of the new tree node
     */
    int32_t newTree();

    /** @brief array of tree nodes */
    OctalTree * trees;
    /** @brief Boid container */
    const BoidContainer * boids;

    /** @brief Number of allocated tree nodes */
    int32_t size;
    /** @brief Number of filled tree nodes */
    int32_t filled;
    /** @brief Global bounding box */
    const Box bbox;

    /** @brief Stats, number of traversed nodes */
    int32_t traversed;
    /** @brief Stats, number of traversed nodes during last query */
    int32_t traversedLast;
    /** @brief Stats, number of queries */
    int32_t queries;
    /** @brief Stats, maximal number of traversed nodes during queries */
    int32_t maxTraversed;

    /** @brief List of boid indicies found during last query */
    int32_t rangeStack[MAXFOUND];
    /** @brief Number of found bods during last querz */
    uint32_t rangeCount;
};