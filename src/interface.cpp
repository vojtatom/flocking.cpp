#include <glm/glm.hpp>
#include <random>
#include "engine.hpp"
#include "boid.hpp"


Interface::Interface(SimulationEngine * _engine)
: engine(_engine), mouse(false), x(0), y(0), deltaX(0), deltaY(0), deltaWheel(0)
{}

void Interface::down(int key)
{
    keymap[key] = true;
    //cout << "Key down: " << key << endl;

    if (keymap[32]) // space
    {
        engine->run = !engine->run;
    }
}

void Interface::up(int key)
{
    keymap[key] = false;
    //cout << "Key up: " << key << endl;
}

void Interface::mouseDown(int _x, int _y)
{
    mouse = true;
    x = _x, y = _y;
    deltaX = 0, deltaY = 0;
}

void Interface::mouseUp(int _x, int _y)
{
    mouse = false;
    x = _x, y = _y;
    deltaX = 0, deltaY = 0;
}

void Interface::mouseMove(int _x, int _y)
{
    deltaX = _x - x;
    deltaY = _y - y;
}

void Interface::mouseWheel(int delta)
{
    deltaWheel += delta * 10;
}

void Interface::resize(int w, int h)
{
    engine->context->camera.resize(w, h);
}

void Interface::updateContext(){
    if (keymap['q'])
    {
        engine->context->camera.move(CameraMoveDirection::Z, 1.0);
    }

    if (keymap['e'])
    {
        engine->context->camera.move(CameraMoveDirection::Z, -1.0);
    }

    if (keymap['t'])
    {
        engine->context->camera.set(0, 0, 100);
    }

    if (deltaX != 0 || deltaY != 0)
    {
        engine->context->camera.rotate(deltaX, deltaY);
        x += deltaX, y += deltaY;
        deltaX = 0, deltaY = 0;
    }

    if (deltaWheel != 0)
    {
        engine->context->camera.move(CameraMoveDirection::Z, deltaWheel * 10);
        deltaWheel = 0;
    }
}