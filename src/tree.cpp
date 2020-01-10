#include <glm/gtx/norm.hpp>
#include <algorithm>
#include "box.hpp"
#include "tree.hpp"
#include "utils.hpp"

#define NONE -1

using namespace std;

OctalTree::OctalTree() 
{
    reset();
}

void OctalTree::reset()
{
    for (size_t i = 0; i < SUBTREES; i++)
        subtree[i] = NONE;

    for (size_t i = 0; i < CHILDPERNODE; i++)
        boidIdx[i] = NONE;

    filled = 0;
}

void OctalTree::set(const Box & box){
    bbox = box;
}

void OctalTree::set(const Box & box, unsigned char treeIdx){
    bbox = box;

    if (treeIdx >= 4) 
        bbox.low.x += (bbox.high.x - bbox.low.x) / 2.0f,
        treeIdx -= 4;
    else 
        bbox.high.x -= (bbox.high.x - bbox.low.x) / 2.0f;

    if (treeIdx >= 2) 
        bbox.low.y += (bbox.high.y - bbox.low.y) / 2.0f,
        treeIdx -= 2;
    else 
        bbox.high.y -= (bbox.high.y - bbox.low.y) / 2.0f;

    if (treeIdx >= 1) 
        bbox.low.z += (bbox.high.z - bbox.low.z) / 2.0f;
    else 
        bbox.high.z -= (bbox.high.z - bbox.low.z) / 2.0f;

}

bool OctalTree::covered(glm::vec3 point, float rangeSqr)
{
    glm::vec3 closest;
    closest.x = min(max(bbox.low.x, point.x), bbox.high.x);
    closest.y = min(max(bbox.low.y, point.y), bbox.high.y);
    closest.z = min(max(bbox.low.z, point.z), bbox.high.z);
    return dist2(point, closest) < rangeSqr;
}

#define INITSIZE 32

OctalTreeContainer::OctalTreeContainer(const Box & box, const BoidContainer * _boids)
: boids(_boids), bbox(box)
{
    trees = new OctalTree[INITSIZE];
    size = INITSIZE;
    reset();
}

OctalTreeContainer::~OctalTreeContainer()
{
    delete [] trees;
}

void OctalTreeContainer::insert(const Boid & boid)
{   
    insert(boid, 0);
}

void OctalTreeContainer::insert(const Boid & boid, int32_t idx)
{   
    //in leaf
    if (trees[idx].filled != CHILDPERNODE)
    {
        trees[idx].boidIdx[trees[idx].filled++] = boid.id;
        return;
    }

    //in interior
    float cx = (trees[idx].bbox.low.x + trees[idx].bbox.high.x) / 2.0f;
    float cy = (trees[idx].bbox.low.y + trees[idx].bbox.high.y) / 2.0f;
    float cz = (trees[idx].bbox.low.z + trees[idx].bbox.high.z) / 2.0f;
    
    int32_t subtreeIdx = 0;
    subtreeIdx += boid.position.x > cx ? 4 : 0;
    subtreeIdx += boid.position.y > cy ? 2 : 0;
    subtreeIdx += boid.position.z > cz ? 1 : 0;

    //build interior tree
    int32_t gsubtreeIdx = trees[idx].subtree[subtreeIdx];
    if (gsubtreeIdx == NONE)
    {
        gsubtreeIdx = newTree();
        trees[idx].subtree[subtreeIdx] = gsubtreeIdx;
        trees[gsubtreeIdx].set(trees[idx].bbox, subtreeIdx);
    }
    
    //propagate lower
    insert(boid, gsubtreeIdx);
}

OctalTree * OctalTreeContainer::getTrees(int32_t & count) const
{
    count = filled;
    return trees;
}

void OctalTreeContainer::findRange(glm::vec3 point, float range)
{
    rangeCount = 0;
    traversedLast = 0;
    findRange(point, range * range, 0);
    traversed += traversedLast;
    maxTraversed = max(maxTraversed, traversedLast);
    queries++;
}

void OctalTreeContainer::findRange(glm::vec3 point, float rangeSqr, int32_t idx)
{
    traversedLast++;

    int32_t lidx;
    for (int32_t i = 0; i < trees[idx].filled; i++)
    {
        lidx = trees[idx].boidIdx[i];
        if (dist2(boids->boids[lidx].position, point) <= rangeSqr)
            rangeStack[rangeCount++] = lidx;
    }

    for (int32_t i = 0; i < SUBTREES; i++)
    {
        if ((lidx = trees[idx].subtree[i]) != NONE)
            if (trees[lidx].covered(point, rangeSqr))
                findRange(point, rangeSqr, lidx);
    }
}

const int32_t * OctalTreeContainer::getBoidsInRange(uint32_t & count) const
{
    count = rangeCount;
    return rangeStack;
}

const glm::vec3 & OctalTreeContainer::getBoxLow() const
{
    return bbox.low;
}

const glm::vec3 & OctalTreeContainer::getBoxHigh() const
{
    return bbox.high;
}

const glm::vec3 OctalTreeContainer::getBoxDiagonal() const
{
    return bbox.high - bbox.low;
}

int32_t OctalTreeContainer::newTree()
{
    if (filled < size)
    {
        trees[filled].reset();
        return filled++;
    }

    //realloc
    OctalTree * tmp = trees;
    trees = new OctalTree[2 * size];

    for (int32_t i = 0; i < filled; i++)
        trees[i] = tmp[i];

    delete [] tmp;
    trees[filled].reset();
    size *= 2;
    return filled++;
}

void OctalTreeContainer::reset() {
    trees[0].reset();
    trees[0].bbox = bbox;
    filled = 1;
    traversed = 0;
    queries = 0;
    maxTraversed = 0;
}

void OctalTreeContainer::printStats()
{
    cout << "Traversed:     " << traversed << endl;
    cout << "Queries:       " << queries << endl;
    cout << "Max Traversed: " << maxTraversed << endl;
    cout << "Avg Traversed: " << float(traversed) / float(queries) << endl;
}