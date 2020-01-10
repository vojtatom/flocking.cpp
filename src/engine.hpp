#pragma once
#include <glm/glm.hpp>
#include "boid.hpp"
#include "tree.hpp"
#include "glcontext.hpp"
#include "environment.hpp"
#include "interface.hpp"


class SimulationEngine
{
public:
    SimulationEngine(Environment * env);
    virtual ~SimulationEngine();
    virtual void draw() = 0;
    virtual void update() = 0;

    //subclasses
    Interface interface;

protected:
    void flockIndividual(Boid & me, Boid & other);
    void flockApply(Boid & me);
    void flockInit();
    void updateBoid(Boid & me);

    friend class Interface;

    BoidContainer *agents;
    Context *context;
    Environment * env;

    //space setting
    glm::vec3 lowBorder, highBorder;
    size_t boidCount;
    glm::vec3 diagonal;

    //flocking constants
    float range;
    float range2;

    float speedFactor;
    float forceLimit;

    //flocking
    glm::vec3 align;
    glm::vec3 cohesion;
    glm::vec3 separation;
    glm::mat4 view;

    size_t alignCount;
    size_t cohesCount;
    size_t separCount;
    size_t viewCount;

    bool run;
};


class EngineCPUBasic : public SimulationEngine
{
public:
    EngineCPUBasic(Environment * env);
    virtual ~EngineCPUBasic();

    virtual void draw();
    virtual void update();
protected:
    void flock(Boid & me);
};


class EngineCPUTree : public SimulationEngine
{
public:
    EngineCPUTree(Environment * env);
    virtual ~EngineCPUTree();

    virtual void draw();
    virtual void update();
protected:
    void flock(Boid & me);

    OctalTreeContainer * tree;
};

class EngineGPUBasic : public SimulationEngine
{
public:
    EngineGPUBasic(Environment * env);
    virtual ~EngineGPUBasic();

    virtual void draw();
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

class EngineGPUGrid: public SimulationEngine
{
public:
    EngineGPUGrid(Environment * env);
    virtual ~EngineGPUGrid();

    virtual void draw();
    virtual void update();
protected:

};

