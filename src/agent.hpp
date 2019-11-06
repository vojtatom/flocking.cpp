#pragma once
#include <cstddef>
#include <glm/glm.hpp>
#include <iostream>
#include "dna.hpp"

using namespace std;

struct Agent{
    glm::vec3 position;
    glm::vec3 velocity;  
    glm::vec3 acceleration;  
    DNA dna;
};

struct AgentContainer
{
    Agent * boids;
    size_t size;
};

ostream & operator<<(ostream & os, const glm::vec4 & v);
ostream & operator<<(ostream & os, const glm::vec3 & v);
ostream & operator<<(ostream & os, const AgentContainer & agents);
