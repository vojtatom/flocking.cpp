#pragma once
#include <glm/glm.hpp>

enum CameraMoveDirection
{
    X,
    Y,
    Z
};

struct Camera
{
    Camera(glm::vec3 _pos, glm::vec3 _center, glm::vec3 _up);

    void resize(int w, int h);
    void move(CameraMoveDirection dir, float delta);
    void rotate(int deltaX, int deltaY);
    void frame();

    glm::vec3 pos;
    glm::vec3 center;
    glm::vec3 up;

    glm::vec3 sPos;
    glm::vec3 sCenter;
    glm::vec3 sUp;

    float posMomentum;
    float rotMomentum;
    float centerMomentum;

    glm::mat4 view;
    glm::mat4 projection;

    float fovy;
    float aspect;
    float near;
    float far;
};
