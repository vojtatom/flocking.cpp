#pragma once
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include "agent.hpp"
#include "camera.hpp"

using namespace std;

struct Program
{
    GLuint id;
    unordered_map<string, GLuint> attr;
    unordered_map<string, GLuint> unif;
};

class Context
{
public:
    Context(AgentContainer *agents);
    ~Context();

    void setup();
    void draw();

    //subclasses
    Camera camera;

private:
    void loadShader(const char *path, string &contents);
    GLuint loadCompileShader(const char *path, GLenum shaderType);
    GLuint createProgram(const char *path_vert_shader, const char *path_frag_shader);
    void initPrograms();
    void initBuffers();
    void setupBuffers();
    void drawAgents();

    GLuint vao;
    GLuint posBuffer;
    GLuint dirBuffer;
    GLuint velBuffer;

    Program agentProgram;

    AgentContainer *agents;
};
