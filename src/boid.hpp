#pragma once
#include <cstddef>
#include <glm/glm.hpp>
#include <iostream>

using namespace std;

struct Boid{
    glm::vec3 position;
    glm::vec3 velocity;    
    glm::vec3 acceleration;   
    uint32_t id;
    float countAround;
};


#define HISTORY 20

struct BoidContainer
{
    BoidContainer()
    {
        for (size_t i = 0; i < HISTORY; i++)
            mins[i] = 0, maxs[i] = 0;
        
        statIdx = 0;
        partialSumMin = 0;
        partialSumMax = 0;
    }

    Boid * boids;
    uint32_t size;
    
    void updateStats(float minCount, float maxCount)
    {
        float oldMin = mins[statIdx], oldMax = maxs[statIdx];
        mins[statIdx] = minCount, maxs[statIdx] = maxCount;
        partialSumMin -= oldMin, partialSumMax -= oldMax;
        partialSumMin += minCount, partialSumMax += maxCount;
        
        boidMaxCount = partialSumMax / float(HISTORY);
        boidMinCount = partialSumMin / float(HISTORY),
        //boidMinCount = boidMaxCount - boidMaxCount / 2.0f;

        statIdx = (statIdx + 1) % HISTORY;
    }

    float mins[HISTORY];
    float maxs[HISTORY];
    float partialSumMin;
    float partialSumMax;
    size_t statIdx;

    float boidMaxCount;
    float boidMinCount;
};

ostream & operator<<(ostream & os, const glm::vec4 & v);
ostream & operator<<(ostream & os, const glm::vec3 & v);
ostream & operator<<(ostream & os, const BoidContainer & agents);
