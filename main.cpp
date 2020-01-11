#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/vec2.hpp>
#include <iostream>
#include <cstdlib>
#include "src/engine.hpp"
#include "src/environment.hpp"
#include "src/boid.hpp"

using namespace std;

/// Main windows parameters
#define TITLE "Flocking"
#define WIDTH 500
#define HEIGHT 500

static SimulationEngine *engine = nullptr;
static int window;

void myDisplay(void)
{
    if (engine != nullptr)
        engine->draw();
    // swap buffers (float buffering)
    glutSwapBuffers();
}

/// called upon window size change
void myReshape(int width, int height)
{
    glViewport(0, 0, width, height);

    if (engine != nullptr)
        engine->interface.resize(width, height);
}

/// Callback for processing special key input
void mySpecialDown(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_LEFT:
    case GLUT_KEY_UP:
    case GLUT_KEY_RIGHT:
    case GLUT_KEY_DOWN:
    case GLUT_KEY_PAGE_UP:
    case GLUT_KEY_PAGE_DOWN:
        engine->interface.down(key);
        break;
    }

    glutPostRedisplay();
}

void mySpecialUp(int key, int x, int y)
{
    if (engine == nullptr)
        return;

    switch (key)
    {
    case GLUT_KEY_LEFT:
    case GLUT_KEY_UP:
    case GLUT_KEY_RIGHT:
    case GLUT_KEY_DOWN:
    case GLUT_KEY_PAGE_UP:
    case GLUT_KEY_PAGE_DOWN:
        engine->interface.up(key);
        break;
    }

    glutPostRedisplay();
}

/// Callback for processing keyboard input
void myKeyboardDown(unsigned char key, int x, int y)
{

    switch (key)
    {
    case 27: //escape
        if (engine != nullptr)
            delete engine;
        glutDestroyWindow(window);
        break;
    default:
        if (engine != nullptr)
            engine->interface.down(key);
        break;
    }
}

void myKeyboardUp(unsigned char key, int x, int y)
{
    if (engine == nullptr)
        return;

    switch (key)
    {
    default:
        engine->interface.up(key);
        break;
    }
}

void myMouse(int button, int state, int x, int y)
{
    if (engine == nullptr)
        return;

    if (button == 0) //left click
    { 
        switch (state)
        {
        case 0: //down
            engine->interface.mouseDown(x, y);
            break;
        case 1: //up
            engine->interface.mouseUp(x, y);
            break;
        }
    }

    if (button == 3) //mouse scroll
    {
        engine->interface.mouseWheel(-1);
    }

    if (button == 4) // mouse scroll other direction
    {
        engine->interface.mouseWheel(1);
    }
    
    
    //cout << button << " " << state << " " << x << " " << y << endl;
}

void myMouseMove(int x, int y)
{
    if (engine == nullptr)
        return;
    //calls with mouse down
    //cout << x << " " << y << endl;
    engine->interface.mouseMove(x, y);
}

void timer(int value)
{
    glutTimerFunc(16, timer, 0);
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    window = glutCreateWindow(TITLE);
    glutInitWindowPosition(5, 5);
    glutInitWindowSize(WIDTH, HEIGHT);

    //Initn GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (err != GLEW_OK)
    {
        // Problem: glewInit failed, something is seriously wrong.
        cout << "glewInit failed: " << glewGetErrorString(err) << endl;
        exit(1);
    }
    cout << "Using GLEW Version: " << glewGetString(GLEW_VERSION) << endl;


    Environment * env = new Environment(argc, argv);


    //simulation init
    switch (env->getInt("appType"))
    {
    case 0:
        engine = new EngineCPUBasic(env);
        break;
    case 1:
        engine = new EngineCPUTree(env);
        break;
    case 2:
        engine = new EngineGPUBasic(env);
        break;
    /*case 3:
        engine = new EngineGPUTree(env);
        break;*/
    case 3:
        engine = new EngineGPUGrid(env);
        break;
    default:
        cout << "USAGE: ./flocking enviroment [app type] [number of boids]" << endl;
        cout << "        enviroment:      relative path to enviroment file with repspect to application location (env.txt)" << endl;
        cout << "        app type:        0  naive CPU, can handle well 2^10 boids" << endl;
        cout << "                         1  octree CPU, can handle well 2^12 boids" << endl;
        cout << "                         2  naive GPU, can handle well 2^15 boids" << endl;
        cout << "                         3  grid GPU, can handle well 2^17 boids" << endl;
        cout << "        number of boids: number of boids given as the exponent of 2" << endl;
        cout << "                         e.g.: 10 spawns 1024 boids" << endl;
        cout << "                               17 spawns 131 072 boids" << endl;
        cout << "Unknown app type, ending..." << endl;
        delete env;
        break;
    }


    // Create main window and set callbacks
    glutTimerFunc(0, timer, 0);
    glutDisplayFunc(myDisplay);
    glutReshapeFunc(myReshape);
    glutKeyboardFunc(myKeyboardDown);
    glutKeyboardUpFunc(myKeyboardUp);
    glutSpecialFunc(mySpecialDown);
    glutSpecialUpFunc(mySpecialUp);
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    glutMouseFunc(myMouse);
    glutMotionFunc(myMouseMove);
    glutReshapeWindow(800, 600);

    glutMainLoop();
    return 0;
}
