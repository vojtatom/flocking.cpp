#pragma once
#include <glm/vec3.hpp>
#include "agent.hpp"
#include "drawing.hpp"
#include "interface.hpp"

class SimulationEngine
{
public:
    SimulationEngine(size_t agentCount, const glm::vec3 spaceBoundLow, glm::vec3 spaceBoundHigh);
    ~SimulationEngine();
    void draw();
    void update();

    //subclasses
    Interface interface;
private:
    void flock(const size_t me, size_t * boids, size_t lboids);
    friend class Interface;

    AgentContainer *agents;
    Context *context;
    glm::vec3 lowBorder, highBorder;
};
