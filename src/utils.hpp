#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/norm.hpp>

using namespace std;

inline float angle(const glm::vec3 & a, const glm::vec3 & b);

inline glm::vec3 limit(const glm::vec3 & v, float limit);

inline float dist2(const glm::vec3 & a, const glm::vec3 & b);

inline float len2(const glm::vec3 & a);

inline float angle(const glm::vec3 & a, const glm::vec3 & b){
    float dot = glm::dot(a, b);
    float len = glm::length(a) * glm::length(b);
    float dl = dot / len;
    float an = acos(fmax(-1.0f, fmin(dl, 1.0f))); //minmax to prevent errors
    return an;
}

inline glm::vec3 limit(const glm::vec3 & v, float limit){
    if (len2(v) > limit * limit){
        return glm::normalize(v) * limit;
    }
    return v;
}

inline float dist2(const glm::vec3 & a, const glm::vec3 & b)
{
    float x = a.x - b.x;
    float y = a.y - b.y;
    float z = a.z - b.z;
    return x * x + y * y + z * z;
}

inline float len2(const glm::vec3 & a)
{
    return a.x * a.x + a.y * a.y + a.z * a.z;
}