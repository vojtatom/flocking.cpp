#pragma once
#include <cstddef>
#include <glm/glm.hpp>
#include <iostream>

using namespace std;

/**
 * @brief Boid structure
 * 
 * Structure representing a single boid
 */
struct Boid{
    /**
     * @brief Spatial position
     */
    glm::vec3 position;
    /**
     * @brief Boid velocity
     */
    glm::vec3 velocity;
    /**
     * @brief Momentary acceleration
     */
    glm::vec3 acceleration;   
    /**
     * @brief Boid ID
     * Also used to store ID of the cell if grid-based app is used
     */
    uint32_t id;
    /**
     * @brief Number of boids within the flocking range around the boid
     */
    float countAround;
};

#define HISTORY 100

/**
 * @brief Helper structure storing boids and adiitional statistics
 */
struct BoidContainer
{
    /**
     * @brief Construct a new Boid Container object
     * Sets all to null by default, all values have to be set manually
     */
    BoidContainer()
    {
        for (size_t i = 0; i < HISTORY; i++)
            mins[i] = 0, maxs[i] = 0;
        
        statIdx = 0;
        partialSumMin = 0;
        partialSumMax = 0;
    }

    /**
     * @brief Array of boids
     */
    Boid * boids;
    /**
     * @brief Number of element in the container
     */
    uint32_t size;
    
    /**
     * @brief Update the statistics stored in the container
     * The container calculates the average of minimal and maximal number seen in range,
     * the average is calculated from last N iterations (defined by macro HISTORY), the update
     * of the average is calculaten in O(1)
     * 
     * @param minCount New minimal value
     * @param maxCount New maximal value
     */
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

    /**
     * @brief Array of last N minimal values
     */
    float mins[HISTORY];
    /**
     * @brief Array of last N maximal values
     */
    float maxs[HISTORY];
    /**
     * @brief Intermediate sum of minimal values
     */
    float partialSumMin;
    /**
     * @brief Intermediate sum of maximal values
     */
    float partialSumMax;
    /**
     * @brief Current index in the arrays of statistical values
     * 
     */
    size_t statIdx;

    /**
     * @brief Average maximal number of boids in range
     */
    float boidMaxCount;
    /**
     * @brief Average minimal number of boids in range
     */
    float boidMinCount;
};

ostream & operator<<(ostream & os, const glm::vec4 & v);
ostream & operator<<(ostream & os, const glm::vec3 & v);
ostream & operator<<(ostream & os, const glm::uvec3 & v);
ostream & operator<<(ostream & os, const BoidContainer & agents);
