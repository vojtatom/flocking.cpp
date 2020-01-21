#pragma once
#include <glm/glm.hpp>
#include "boid.hpp"
#include "tree.hpp"
#include "glcontext.hpp"
#include "environment.hpp"
#include "interface.hpp"
#include <chrono>

#define TIMEHIST 200

/**
 * @brief Timer class, allows to measure time
 * 
 * Designed for repeated use and measurement of the average time
 */
class Timer {
public:
    /**
     * @brief Construct a new Timer object
     * 
     * Inits all internals to null
     */
    Timer(){
        timeIndex = 0;
        for (size_t i = 0; i < TIMEHIST; i++)
            times[i] = 0;
        sumTime = 0;
        allFilled = false;
    };

    /**
     * @brief Start the timer
     */
    void startTime()
    {
        timepoint = Clock::now();
    };
    
    /**
     * @brief Stop the timer and calculate the average time
     * 
     * Calculates the average time from last N samples (number N defined by #TIMEHIST macro),
     * updates the time in O(1)
     */
    void stopTime()
    {
        auto endTime = Clock::now();
        int oldTime = times[timeIndex];
        int duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - timepoint).count();
        
        sumTime -= oldTime;
        sumTime += duration;

        meanTime = float(sumTime) / TIMEHIST;

        times[timeIndex] = duration;
        
        if (timeIndex == TIMEHIST - 1)
            allFilled = true;

        timeIndex = (timeIndex + 1) % TIMEHIST;

        if (allFilled)
            std::cout << "\rUpdate speed: " << meanTime << " ms, last: " << duration << "ms" <<  std::flush;
        else 
            std::cout << "\rTesting: " << float(timeIndex) * 100.0f / TIMEHIST << std::flush;
    }

private:
    /**
     * @brief Clock from <chrono>
     */
    typedef std::chrono::high_resolution_clock Clock;

    #ifdef __linux__
        std::chrono::system_clock::time_point timepoint;
    #elif _WIN32
        std::chrono::steady_clock::time_point timepoint;
    #else
        std::chrono::system_clock::time_point timepoint;
    #endif

    /**
     * @brief Average time
     */
    float meanTime;

    /**
     * @brief Sum of all emasured times
     */
    int sumTime;

    /**
     * @brief Array of last N measured times
     */
    int times[TIMEHIST];

    /**
     * @brief Flag to indicate if the array has been at least once fully filled
     */
    bool allFilled;

    /**
     * @brief Index of the current time in the Timer::times array
     */
    size_t timeIndex;
};


/**
 * @brief Base class for all simulation engines
 * 
 * Encapsulates the logic for flocking simulation, 
 * manages graphics context, containers, trees, etc.
 */
class SimulationEngine
{
public:
    /**
     * @brief Construct a new Simulation Engine object
     * 
     * @param env Environment parameter with valid user settings
     */
    SimulationEngine(Environment * env);

    /**
     * @brief Destroy the Simulation Engine object
     */
    virtual ~SimulationEngine();

    /**
     * @brief Method for updating the screen
     */
    virtual void draw() = 0;

    /**
     * @brief Method implementing the single iteration update procedure
     */
    virtual void update() = 0;

    /**
     * @brief Interface subclass, manages the user inputs, mouse, keyboard...
     */
    Interface interface;

protected:
    /**
     * @brief CPU implementation of flocking
     * 
     * Add the influence of the other
     * the function does not check for proximity and always 
     * performs the flocking algorithm, accumulate forces
     * 
     * @param me Boid influenced by other 
     * @param other Boid incluencing me
     */
    void flockIndividual(Boid & me, Boid & other);

    /**
     * @brief Apply the accumulated forces to the boid
     * 
     * @param me Boid to be modified by the accumulated forces
     */
    void flockApply(Boid & me);

    /**
     * @brief Init variables for forces accumulation
     * Performed before a single boid flocking
     */
    void flockInit();

    /**
     * @brief Update boid based on its internal settings
     * 
     * Applies the acceleration to the velocitz and modifies the position.
     * 
     * @param me Boid to be updated
     */
    void updateBoid(Boid & me);


    //to allow for graphics context modification
    friend class Interface;

    /**
     * @brief Boids container
     */
    BoidContainer *agents;

    /**
     * @brief Grahpic context
     */
    Context *context;

    /**
     * @brief User settings 
     */
    Environment * env;

    /**
     * @brief Low and high borders of the flocking space
     */
    glm::vec3 lowBorder, highBorder;

    /**
     * @brief Number of boids
     */
    size_t boidCount;

    /**
     * @brief Diagonal dimensions of the flocking space
     */
    glm::vec3 diagonal;

    /**
     * @brief Flocking range
     */
    float range;

    /**
     * @brief Flocking range squared
     */
    float range2;

    /**
     * @brief Speed factor used in flocking algo.
     */
    float speedFactor;

    /**
     * @brief Force limit used in the flocking alog.
     */
    float forceLimit;

    /**
     * @brief Alignment, flocking algorithm accumulator for currently processed single boid
     */
    glm::vec3 align;
    /**
     * @brief Cohesion, flocking algorithm accumulator for currently processed single boid
     */
    glm::vec3 cohesion;
    /**
     * @brief Separation, flocking algorithm accumulator for currently processed single boid
     */
    glm::vec3 separation;
    /**
     * @brief Rotation matrix for view, flocking algorithm accumulator for currently processed single boid
     */
    glm::mat4 view;

    /**
     * @brief Number of boids in range, acumulator for currently processed boid 
     */
    size_t flockingCount;
    /**
     * @brief Number of boids obstruction view, acumulator for currently processed boid 
     */
    size_t viewCount;

    /**
     * @brief Flag for pausing the simulation
     */
    bool run;
};

/**
 * @brief Engine implementing naive flocking
 * 
 * Runs on CPU, complexity of flocking alogrithm is O(n^2)
 */
class EngineCPUBasic : public SimulationEngine
{
public:
    /**
     * @brief Construct a new engine
     * 
     * @param env Environment parameter with valid user settings
     */
    EngineCPUBasic(Environment * env);
    
    /**
     * @brief Destroy the engine
     */
    virtual ~EngineCPUBasic();

    /**
     * @brief Method for updating the screen
     */
    virtual void draw();

    /**
     * @brief Update procedure
     * 
     * Iterates through boids and checks for distance of each two
     */
    virtual void update();
protected:

    /**
     * @brief Naive implementaiton of flocking for a single boid
     * 
     * @param me Boid to be processed
     */
    void flock(Boid & me);
};

/**
 * @brief Engine implementing flocking with octree
 * 
 * Runs on CPU, complexity of flocking alogrithm is O(n log(n)), uses octree
 */
class EngineCPUTree : public SimulationEngine
{
public:
    /**
     * @brief Construct a new engine
     * 
     * @param env Environment parameter with valid user settings
     */
    EngineCPUTree(Environment * env);
    
    /**
     * @brief Destroy the engine
     */
    virtual ~EngineCPUTree();

    /**
     * @brief Method for updating the screen
     */
    virtual void draw();

    /**
     * @brief Update procedure
     * 
     * Iterates through boids and checks for distance of each two
     */
    virtual void update();
protected:
    /**
     * @brief Improved implementaiton of flocking for a single boid
     * Uses lookup in octree to obtain close boids
     * 
     * @param me Boid to be processed
     */
    void flock(Boid & me);

    /**
     * @brief Octree used to store boids
     */
    OctalTreeContainer * tree;
};

/**
 * @brief Engine implementing flocking on GPU, naive
 * 
 * Runs on GPU, no further optimizations, each boid is processed in parallel
 */
class EngineGPUBasic : public SimulationEngine
{
public:
    /**
     * @brief Construct a new engine
     * 
     * @param env Environment parameter with valid user settings
     */
    EngineGPUBasic(Environment * env);

    /**
     * @brief Destroy the engine
     */
    virtual ~EngineGPUBasic();

    /**
     * @brief Method for updating the screen
     */
    virtual void draw();

    /**
     * @brief Update procedure
     * 
     * Updates and flocking are performed using compute shaders 
     * in graphic context. Each boid tests all other boids,
     * which is not really ideal.
     */
    virtual void update();
protected:
};

/*class EngineGPUTree: public SimulationEngine
{
public:
    EngineGPUTree(Environment * env);
    virtual ~EngineGPUTree();

    virtual void draw();
    virtual void update();
protected:

    OctalTreeContainer * tree;
};*/

/**
 * @brief Engine implementing flocking on GPU, uses grid
 * 
 * Utilizes grid to order boids and limit the number of tested boids.
 * The size of the grid is defined by the flocking radius, 
 * size of the cell border is double the flocking range. 
 */
class EngineGPUGrid: public SimulationEngine
{
public:
    /**
     * @brief Construct a new engine
     * 
     * @param env Environment parameter with valid user settings
     */
    EngineGPUGrid(Environment * env);

    /**
     * @brief Destroy the engine
     */
    virtual ~EngineGPUGrid();

    /**
     * @brief Method for updating the screen
     */
    virtual void draw();

    /**
     * @brief Update procedure
     * 
     * Updates and flocking are performed using compute shaders 
     * in graphic context. Each boid tests all boids 
     * in the same call and the surrounding 8 cells.
     */
    virtual void update();
protected:

};

