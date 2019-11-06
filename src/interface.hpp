#pragma once
#include <unordered_map>

class SimulationEngine;

class Interface{
public:
    Interface(SimulationEngine * _engine);
    void resize(int w, int h);
    void down(int key);
    void up(int key);
    void mouseDown(int x, int y);
    void mouseUp(int x, int y);
    void mouseMove(int x, int y);
    void mouseWheel(int delta);
    void updateContext();

private:
    SimulationEngine * engine;

    unordered_map<int, bool> keymap;
    bool mouse;
    int x, y, deltaX, deltaY, deltaWheel;
};