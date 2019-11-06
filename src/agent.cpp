#include <iostream>
#include <glm/glm.hpp>
#include "agent.hpp"


ostream & operator<<(ostream & os, const glm::vec4 & v)
{
    os << v.x << " " << v.y << " " << v.z << " " << v.w;
    return os;
}

ostream & operator<<(ostream & os, const glm::vec3 & v)
{
    os << v.x << " " << v.y << " " << v.z;
    return os;
}

ostream & operator<<(ostream & os, const AgentContainer & agents){
    for (size_t i = 0; i < agents.size; i++)
    {
        os << "Agent " << agents.boids[i].position << "\n      ";
        os << agents.boids[i].velocity << "\n";
    }
    return os;
}