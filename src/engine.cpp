#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <random>
#include "engine.hpp"
#include "drawing.hpp"
#include "agent.hpp"
#include "interface.hpp"
#include "dna.hpp"
#include "utils.hpp"

#define foodZone 200
#define alignmentZone 40
#define cohesionZone 40
#define seperationZone 40
#define viewZone 40
#define closeZone 20
#define happyZone 40
#define sensitiveZone 40

using namespace std;

SimulationEngine::SimulationEngine(size_t agentCount, const glm::vec3 spaceBoundLow, glm::vec3 spaceBoundHigh)
    : interface(this), lowBorder(spaceBoundLow), highBorder(spaceBoundHigh)
{
    Agent *boids = new Agent[agentCount];

    random_device randomDevice;
    mt19937 generator(randomDevice());

    uniform_real_distribution<float> disX(spaceBoundLow.x, spaceBoundHigh.x);
    uniform_real_distribution<float> disY(spaceBoundLow.y, spaceBoundHigh.y);
    uniform_real_distribution<float> disZ(spaceBoundLow.z, spaceBoundHigh.z);
    uniform_real_distribution<float> disV(0, 0.1f);
    uniform_real_distribution<float> dnaDist(0, 1.0f);

    //init agents
    for (size_t i = 0; i < agentCount; i++)
    {
        boids[i].position.x = disX(generator),
        boids[i].position.y = disY(generator),
        boids[i].position.z = disZ(generator),
        boids[i].velocity.x = disX(generator),
        boids[i].velocity.y = disY(generator),
        boids[i].velocity.z = disZ(generator);
        boids[i].velocity = glm::normalize(boids[i].velocity) * disV(generator);
        boids[i].dna.set(dnaDist, generator);
    }

    agents = new AgentContainer{boids, agentCount};
    context = new Context(agents);
    context->setup();
}

SimulationEngine::~SimulationEngine()
{
    delete context;
    delete[] agents->boids;
    delete agents;
}

void SimulationEngine::draw()
{
    //update values
    update();
    context->setup();
    context->draw();

    //handle UI
    interface.updateContext();
}

#define RANGE 1.0f

void SimulationEngine::update()
{
    glm::vec3 diagonal = highBorder - lowBorder;

    //update boids positions and velocities
    for (size_t i = 0; i < agents->size; i++)
    {
        agents->boids[i].position = glm::mod(((agents->boids[i].position + agents->boids[i].velocity) - lowBorder + diagonal), diagonal) + lowBorder;
        agents->boids[i].velocity = agents->boids[i].velocity + agents->boids[i].acceleration;
        agents->boids[i].acceleration = glm::vec3(0.0f);
    }

    static size_t boids[1000];
    static size_t lboids;

    //NAIVE SEARCH
    for (size_t i = 0; i < agents->size; i++)
    {
        lboids = 0;
        for (size_t j = 0; j < agents->size; j++)
        {
            if (glm::length(agents->boids[i].position - agents->boids[j].position) < RANGE){
                boids[lboids++] = j;
                if (lboids == 1000)
                    break;
            }
        }

        flock(i, boids, lboids);
    }
    
}

#define ANGLEDELTA 0.01f

void SimulationEngine::flock(const size_t ime, size_t *boids, size_t lboids)
{
    //local copy of me
    Agent &me = agents->boids[ime];
    Agent other;

    //flocking vars
    static float dist, ang;
    static glm::vec3 align = glm::vec3(0.0f);
    static glm::vec3 cohesion = glm::vec3(0.0f);
    static glm::vec3 separation = glm::vec3(0.0f);
    static glm::mat4 view = glm::mat4(1.0);
    static glm::vec3 tmp;
    static size_t alignCount = 0;
    static size_t cohesCount = 0;
    static size_t separCount = 0;
    static size_t viewCount = 0;

    for (size_t i = 0; i < lboids; i++)
    {
        if (boids[i] == ime)
            continue;

        //setup local boid
        other = agents->boids[boids[i]];
        dist = glm::length(me.position - other.position);

        //alignment
        if (dist < alignmentZone)
        {
            align += other.velocity;
            alignCount++;
        }

        //cohesion
        if (dist < cohesionZone)
        {
            cohesion += other.position;
            cohesCount++;
        }

        //separation
        if (dist < seperationZone)
        {
            separation += (me.position - other.position) / dist;
            separCount++;
        }

        /*//view
        if (dist < viewZone)
        {
            tmp = other.position - me.position;
            ang = angle(me.velocity, tmp);
            if (ang < 1)
            {
                view = glm::rotate(view, ang * ANGLEDELTA, glm::cross(me.velocity, tmp));
                viewCount++;
            }
        }*/
    }

    //alignment
    if (alignCount > 0)
    {
        me.acceleration += limit(glm::normalize(align) * me.dna.speed - me.velocity, me.dna.force);
    }

    //cohesion
    if (cohesCount > 0)
    {
        me.acceleration += limit(glm::normalize(cohesion / float(cohesCount) - me.position) * me.dna.speed - me.velocity, me.dna.force);
    }

    //separation
    if (separCount > 0)
    {
        me.acceleration += limit(glm::normalize(separation) * me.dna.speed - me.velocity, me.dna.force);
    }

   /* //view
    if (viewCount > 0)
    {
        tmp = view * glm::vec4(me.velocity, 1);
        me.acceleration += limit(glm::normalize(tmp) * me.dna.speed - me.velocity, me.dna.force);
    }*/
}