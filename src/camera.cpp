#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include "camera.hpp"
#include "boid.hpp"
#include "utils.hpp"

using namespace std;

#define DELTA 0.05f

Camera::Camera(glm::vec3 _pos, glm::vec3 _center, glm::vec3 _up)
    : pos(_pos), center(_center), up(_up), sPos(_pos), sCenter(center), sUp(_up), fovy(45), aspect(1), fnear(1), ffar(10000)
{
    projection = glm::perspective(fovy, aspect, fnear, ffar);
    view = glm::lookAt(pos, center, up);
}

void Camera::resize(int w, int h){
    aspect = float(w) / float(h);
    projection = glm::perspective(fovy, aspect, fnear, ffar);
}

void Camera::move(CameraMoveDirection dir, float delta){
    glm::vec3 forw = glm::normalize(center - pos);

    switch(dir){
        case CameraMoveDirection::Z:
            pos += DELTA * forw * delta;
            //center += DELTA * forw * delta;
            break;
    } 

    view = glm::lookAt(pos, center, up);
}

void Camera::set(float x, float y, float z)
{
    pos = glm::vec3(x, y, z);
}

void Camera::rotate(int deltaX, int deltaY) {

    float a_x = -deltaX * DELTA * 0.5; //x to radian?
    float a_y = deltaY * DELTA * 0.5; //y to radian?

    glm::vec3 forw = center - pos;
    glm::vec3 axisX = glm::cross(up, forw);
    glm::mat4 m = glm::mat4(1.0);

    m = glm::rotate(m, a_x, up);
    m = glm::rotate(m, a_y, axisX);

    forw = m * glm::vec4(forw, 1);
    up = m * glm::vec4(up, 1);
    pos = center - forw;
}

void Camera::rotateDirect(float deltaX, float deltaY) {

    float a_x = -deltaX * DELTA * 0.5; //x to radian?
    float a_y = deltaY * DELTA * 0.5; //y to radian?

    glm::vec3 forw = sCenter - sPos;
    glm::vec3 axisX = glm::cross(sUp, forw);
    glm::mat4 m = glm::mat4(1.0);

    m = glm::rotate(m, a_x, sUp);
    m = glm::rotate(m, a_y, axisX);

    forw = m * glm::vec4(forw, 1);
    up = sUp = m * glm::vec4(sUp, 1);
    pos = sPos = sCenter - forw;
    view = glm::lookAt(sPos, sCenter, sUp);
}

#define MAXSPEED 0.1f

void Camera::frame(){
    glm::vec3 tmp = pos - sPos;

    posMomentum = fmin(glm::length(tmp), MAXSPEED * 2.0f);
    if (posMomentum > 0.02f){
        sPos += tmp * posMomentum;
    } else {
        sPos = pos, posMomentum = 0;
    }

    float a = angle(sUp, up);
    rotMomentum =  fmin(a, MAXSPEED * 3.14f);

    if (rotMomentum > 0.02f){
        glm::vec3 axis = glm::cross(sUp, up);
        glm::mat4 m = glm::rotate(glm::mat4(1.0f), rotMomentum, axis);
        sUp = glm::normalize(glm::vec3(m * glm::vec4(sUp, 1)));
    } else {
        sUp = up, rotMomentum = 0;
    }
        
    tmp = center - sCenter;
    centerMomentum = fmin(glm::length(tmp), MAXSPEED * 2.0f);
    if (centerMomentum > 0.02f){
        sCenter += tmp * centerMomentum;
    } else {
        sCenter = center, centerMomentum = 0;
    }

    if (centerMomentum != 0 || rotMomentum != 0 || posMomentum != 0){
        view = glm::lookAt(sPos, sCenter, sUp);
    } else {
        //rotateDirect(0.1, 0.1);
    }
}