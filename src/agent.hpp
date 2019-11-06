#pragma once
#include <cstddef>
#include <glm/glm.hpp>
#include <iostream>

using namespace std;

struct AgentContainer
{
    glm::vec3 * positions;
    glm::vec3 * directions;
    float * velocities;
    size_t size;
};

ostream & operator<<(ostream & os, const glm::vec4 & v);
ostream & operator<<(ostream & os, const glm::vec3 & v);
ostream & operator<<(ostream & os, const AgentContainer & agents);
