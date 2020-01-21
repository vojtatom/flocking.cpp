#pragma once
#include <unordered_map>

class SimulationEngine;

/**
 * @brief Class managing user input channels (mouse, keyboard...)
 */
class Interface{
public:
    /**
     * @brief Construct a new Interface object
     * 
     * @param _engine pointer to engine currently in use
     */
    Interface(SimulationEngine * _engine);

    /**
     * @brief Callback for screen resizing
     * 
     * @param w Screen width
     * @param h Screen height
     */
    void resize(int w, int h);

    /**
     * @brief Callback for key down
     * 
     * @param key Code of the pressed key
     */
    void down(int key);
    
    /**
     * @brief Callback for key up
     * 
     * @param key Code of the released key
     */
    void up(int key);

    /**
     * @brief Callback for mouse clicked down
     * 
     * @param x x mouse coordinate
     * @param y y mouse coordinate
     */
    void mouseDown(int x, int y);

    /**
     * @brief Callback for mouse button released
     * 
     * @param x x mouse coordinate
     * @param y y mouse coordinate
     */
    void mouseUp(int x, int y);

    /**
     * @brief Callback for mouse moved
     * 
     * @param x x mouse coordinate
     * @param y y mouse coordinate
     */
    void mouseMove(int x, int y);

    /**
     * @brief Callback for mouse wheel scrolled
     * 
     * @param delta signed amount of scroll
     */
    void mouseWheel(int delta);

    /**
     * @brief Frame update, updates the engine baased on internal keymap etc.
     */
    void updateContext();

private:
    /** @brief Simulation engine currently in use */
    SimulationEngine * engine;

    /** @brief Keymap storing pressed keys */
    unordered_map<int, bool> keymap;
    /** @brief Flag indicating pressed mouse */
    bool mouse;
    /** @brief Mouse setting variables */
    int x, y, deltaX, deltaY, deltaWheel;
};