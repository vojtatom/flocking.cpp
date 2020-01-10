#pragma once
#include <cstddef>
#include <glm/glm.hpp>

#include "box.hpp"
#include "boid.hpp"

#define CHILDPERNODE 16
#define SUBTREES 8


class OctalTreeContainer;

struct OctalTree {
    OctalTree();
    void reset();
    void set(const Box & box);
    void set(const Box & box,  unsigned char treeIdx);
    bool covered(glm::vec3 point, float rangeSqr);

    
    Box bbox; //24 Bytes
    int32_t subtree[SUBTREES]; //32 Bytes
    int32_t boidIdx[CHILDPERNODE]; //16 Bytes
    int32_t filled; //4 Bytes
}; //80 Bytes in total, could be improved to 64 Bytes


#define MAXFOUND 1024

class OctalTreeContainer {
public:
    OctalTreeContainer(const Box & bbox, const BoidContainer * boids);
    ~OctalTreeContainer();

    void insert(const Boid & boid0);
    void findRange(glm::vec3 point, float range);
    void reset();

    OctalTree * getTrees(int32_t & count) const;
    const int32_t * getBoidsInRange(uint32_t & count) const;
    const glm::vec3 & getBoxLow() const;
    const glm::vec3 & getBoxHigh() const;
    const glm::vec3 getBoxDiagonal() const;

    void printStats();

private:
    void insert(const Boid & boid, int32_t treeIdx);
    void findRange(glm::vec3 point, float rangeSqr, int32_t treeIdx);
    int32_t newTree();

    OctalTree * trees;
    const BoidContainer * boids;

    int32_t size;
    int32_t filled;
    const Box bbox;

    int32_t traversed;
    int32_t traversedLast;
    int32_t queries;
    int32_t maxTraversed;

    int32_t rangeStack[MAXFOUND];
    uint32_t rangeCount;
};