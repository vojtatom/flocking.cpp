#include <glm/glm.hpp>
#include <random>
#include "engine.hpp"
#include "drawing.hpp"
#include "agent.hpp"
#include "interface.hpp"

using namespace std;


SimulationEngine::SimulationEngine(size_t agentCount, const glm::vec3 spaceBoundLow, glm::vec3 spaceBoundHigh)
    : interface(this), lowBorder(spaceBoundLow), highBorder(spaceBoundHigh)
{
    glm::vec3 *positions = new glm::vec3[agentCount];
    glm::vec3 *directions = new glm::vec3[agentCount];
    float *velocities = new float[agentCount];

    random_device randomDevice;
    mt19937 generator(randomDevice());

    uniform_real_distribution<float> disX(spaceBoundLow.x, spaceBoundHigh.x);
    uniform_real_distribution<float> disY(spaceBoundLow.y, spaceBoundHigh.y);
    uniform_real_distribution<float> disZ(spaceBoundLow.z, spaceBoundHigh.z);
    uniform_real_distribution<float> disV(0, 0.1f);

    //init agents
    for (size_t i = 0; i < agentCount; i++)
    {
        positions[i].x = disX(generator),
        positions[i].y = disY(generator),
        positions[i].z = disZ(generator),
        directions[i].x = disX(generator),
        directions[i].y = disY(generator),
        directions[i].z = disZ(generator),
        directions[i] = glm::normalize(directions[i]);
        velocities[i] = disV(generator);
    }

    agents = new AgentContainer{positions, directions, velocities, agentCount};
    context = new Context(agents);
    context->setup();
}

SimulationEngine::~SimulationEngine()
{
    delete context;
    delete[] agents->positions;
    delete[] agents->directions;
    delete[] agents->velocities;
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

void SimulationEngine::update(){
    glm::vec3 diagonal = highBorder - lowBorder;

    for (size_t i = 0; i < agents->size; i++)
    {
        agents->positions[i] = glm::mod(((agents->positions[i] + agents->directions[i] * agents->velocities[i]) - lowBorder + diagonal), diagonal) + lowBorder;
    }
}
