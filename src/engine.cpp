#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/norm.hpp>
#include <random>
#include <chrono>
#include "engine.hpp"
#include "glcontext.hpp"
#include "boid.hpp"
#include "interface.hpp"
#include "utils.hpp"
#include "box.hpp"

using namespace std;

SimulationEngine::SimulationEngine(Environment * _env)
    : interface(this), env(_env),
    run(true)
{
    lowBorder = env->getVec("spaceLow");
    highBorder = env->getVec("spaceHigh");
    diagonal = highBorder - lowBorder;
    boidCount = 1 << env->getInt("boidCount");

    range = env->getFloat("flockingZone"); 
    range2 = range * range;

    speedFactor = env->getFloat("speedFactor"); 
    forceLimit = env->getFloat("forceLimit"); 

    Boid *boids = new Boid[boidCount];

    random_device randomDevice;
    mt19937 generator(randomDevice());

    uniform_real_distribution<float> disX(lowBorder.x, highBorder.x);
    uniform_real_distribution<float> disY(lowBorder.y, highBorder.y);
    uniform_real_distribution<float> disZ(lowBorder.z, highBorder.z);
    uniform_real_distribution<float> dnaDist(0, 1.0f);

    //init agents
    for (size_t i = 0; i < boidCount; i++)
    {
        boids[i].position.x = disX(generator),
        boids[i].position.y = disY(generator),
        boids[i].position.z = disZ(generator),
        boids[i].velocity.x = disX(generator),
        boids[i].velocity.y = disY(generator),
        boids[i].velocity.z = disZ(generator),
        boids[i].velocity = glm::normalize(boids[i].velocity);
        boids[i].acceleration = glm::vec3(0.0f);
        boids[i].id = i;
        boids[i].countAround = 0;
    }

    agents = new BoidContainer();
    agents->boids = boids;
    agents->size = boidCount;
    context = new Context(agents, env);
    context->setupBoids();
}

SimulationEngine::~SimulationEngine()
{
    delete context;
    delete[] agents->boids;
    delete agents;
    delete env;
}

#define ANGLEDELTA 0.01f

void SimulationEngine::flockIndividual(Boid & me, Boid & other)
{
    float dist = glm::length(me.position - other.position);

    align += other.velocity, alignCount++;
    cohesion += other.position, cohesCount++;
    separation += (me.position - other.position) / dist, separCount++;

    glm::vec3 tmp = other.position - me.position;
    float ang = angle(me.velocity, tmp);
    if (ang < 1)
    {
        view = glm::rotate(view, -ang * ANGLEDELTA, glm::cross(me.velocity, tmp));
        viewCount++;
    }
    
}

void SimulationEngine::flockApply(Boid & me)
{
    //alignment
    if (alignCount > 0)
        me.acceleration += limit(glm::normalize(align / float(alignCount)) * 
                           speedFactor - me.velocity, forceLimit);

    //cohesion
    if (cohesCount > 0)
        me.acceleration += limit(glm::normalize(cohesion / float(cohesCount) - me.position) * 
                           speedFactor - me.velocity, forceLimit);

    //separation
    if (separCount > 0)
        me.acceleration += limit(glm::normalize(separation / float(separCount)) * 
                           speedFactor - me.velocity, forceLimit);

    //view
    if (viewCount > 0)
    {
        glm::vec3 tmp = view * glm::vec4(me.velocity, 1);
        me.acceleration += limit(glm::normalize(tmp) * speedFactor - me.velocity, forceLimit);
    }

    me.countAround = viewCount;
    agents->boidMaxCount = max((float) viewCount, agents->boidMaxCount);
    agents->boidMinCount = min((float) viewCount, agents->boidMinCount);
}

void SimulationEngine::flockInit()
{
    align = glm::vec3(0.0f);
    cohesion = glm::vec3(0.0f);
    separation = glm::vec3(0.0f);
    view = glm::mat4(1.0);
    alignCount = 0;
    cohesCount = 0;
    separCount = 0;
    viewCount = 0;
}

void SimulationEngine::updateBoid(Boid & me)
{
    me.position = glm::mod(((me.position + me.velocity) - lowBorder + diagonal), diagonal) + lowBorder;
    me.velocity = me.velocity + me.acceleration;
    me.acceleration = glm::vec3(0.0f);
}

//===============================================================================================

EngineCPUBasic::EngineCPUBasic(Environment * env)
: SimulationEngine(env)
{
    //setup main box once
    context->setupBox(Box{lowBorder, highBorder});
}

EngineCPUBasic::~EngineCPUBasic()
{

}

void EngineCPUBasic::draw()
{
    startTime();
    //update values
    update();
    context->setupBoids();
    context->draw();

    //handle UI
    interface.updateContext();
    stopTime();
}

void EngineCPUBasic::update()
{
    if (!run)
        return;

    agents->boidMaxCount = 0;
    agents->boidMinCount = INFINITY;

    //update boids positions and velocities
    for (size_t i = 0; i < agents->size; i++)
        updateBoid(agents->boids[i]);

    //naive search
    for (size_t i = 0; i < agents->size; i++)
        flock(agents->boids[i]);
}

void EngineCPUBasic::flock(Boid & me)
{
    flockInit();

    for (size_t i = 0; i < agents->size; i++)
    {
        if (agents->boids[i].id == me.id)
            continue;

        if(dist2(me.position, agents->boids[i].position) > range2)
            continue;

        flockIndividual(me, agents->boids[i]);
    }

    flockApply(me);
}

//===============================================================================================

EngineCPUTree::EngineCPUTree(Environment * env)
: SimulationEngine(env)
{
    tree = new OctalTreeContainer(Box{lowBorder, highBorder}, agents);
    context->setupTree(tree);
}

EngineCPUTree::~EngineCPUTree()
{
    delete tree;
}

void EngineCPUTree::draw()
{
    //update values
    startTime();
    update();
    context->setupBoids();
    context->setupTree();
    context->draw();
    tree->printStats();

    //handle UI
    interface.updateContext();
    stopTime();
}

void EngineCPUTree::update()
{
    if (!run)
        return;

    //octree stuff
    tree->reset();

    agents->boidMaxCount = 0;
    agents->boidMinCount = INFINITY;

    //update boids positions and velocities
    for (size_t i = 0; i < agents->size; i++)
    {
        updateBoid(agents->boids[i]);
        tree->insert(agents->boids[i]);
    }

    for (size_t i = 0; i < agents->size; i++)
        flock(agents->boids[i]);
}

void EngineCPUTree::flock(Boid & me)
{
    flockInit();

    tree->findRange(me.position, range);
    //number of found in range
    uint32_t matches;
    const int32_t * neighbors = tree->getBoidsInRange(matches);

    for (size_t i = 0; i < matches; i++)
    {
        if (agents->boids[neighbors[i]].id == me.id)
            continue;

        flockIndividual(me, agents->boids[neighbors[i]]);
    }

    flockApply(me);
}

//===============================================================================================

EngineGPUBasic::EngineGPUBasic(Environment * env)
: SimulationEngine(env)
{
    //setup main box once
    context->setupBox(Box{lowBorder, highBorder});
}

EngineGPUBasic::~EngineGPUBasic()
{}

void EngineGPUBasic::draw()
{
    //update values
    startTime();
    update();
    context->draw();

    //handle UI
    interface.updateContext();
    stopTime();
}

void EngineGPUBasic::update()
{
    if (!run)
        return;

    context->computeShaderUpdateBoids();
    context->computeShaderNaiveFlock();
}

//===============================================================================================

/*EngineGPUTree::EngineGPUTree(Environment * env)
: SimulationEngine(env)
{
    tree = new OctalTreeContainer(Box{lowBorder, highBorder}, agents);
    context->setupTree(tree);
    //context->setupBox(Box{lowBorder, highBorder});
}

EngineGPUTree::~EngineGPUTree()
{
    //delete tree;
}

void EngineGPUTree::draw()
{
    //update values
    update();
    context->setupTree();
    context->draw();

    //handle UI
    interface.updateContext();
}

void EngineGPUTree::update()
{
    if (!run)
        return;

    tree->reset();

    context->computeShaderUpdateBoids();
    context->computeShaderTreeFlock();

    context->copyBoidsToCPU();
    
    //update boids positions and velocities
    for (size_t i = 0; i < agents->size; i++)
        tree->insert(agents->boids[i]);

}*/

//===============================================================================================

EngineGPUGrid::EngineGPUGrid(Environment * env)
: SimulationEngine(env)
{
    context->setupGrid(Box{lowBorder, highBorder});
    //context->setupBox(Box{lowBorder, highBorder});
}

EngineGPUGrid::~EngineGPUGrid()
{

}

void EngineGPUGrid::draw()
{
    //update values
    startTime();
    update();
    context->draw();

    //handle UI
    interface.updateContext();
    stopTime();
}

void EngineGPUGrid::update()
{
    if (!run)
        return;

    context->computeShaderUpdateBoids();
    context->computeShaderSortBoids();
    context->computeShaderGridFlock();
}
