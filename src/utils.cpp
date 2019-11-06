#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/norm.hpp>
#include "utils.hpp"

using namespace std;

float angle(const glm::vec3 a, const glm::vec3 b){
    float dot = glm::dot(a, b);
    float len = glm::length(a) * glm::length(b);
    float dl = dot / len;
    float an = acos(fmax(-1.0f, fmin(dl, 1.0f))); //minmax to prevent errors
    return an;
}

glm::vec3 limit(glm::vec3 v, float limit){
    if (glm::length2(v) > limit * limit){
        v = glm::normalize(v) * limit;
    }
    return v;
}