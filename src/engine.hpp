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
    friend class Interface;

    AgentContainer *agents;
    Context *context;
    glm::vec3 lowBorder, highBorder;
};
