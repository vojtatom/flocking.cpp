#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <string>
#include <fstream>
#include <sstream>

#include "drawing.hpp"
#include "engine.hpp"
#include "agent.hpp"
#include "camera.hpp"


void Context::loadShader(const char *path, string &contents)
{
    ifstream file(path);
    stringstream buffer;

    buffer << file.rdbuf();
    contents = buffer.str();
}

GLuint Context::loadCompileShader(const char *path, GLenum shaderType)
{
    string contents;
    loadShader(path, contents);
    const char *src = &(contents[0]);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint test;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &test);
    if (!test)
    {
        char compilation_log[512];
        glGetShaderInfoLog(shader, 512, NULL, compilation_log);
        cerr << "Shader compilation ERROR: " << compilation_log << endl;

        //TODO not okay solution
        exit(1);
    }
    else
    {
        cout << "Compilation OK" << endl;
    }

    return shader;
}

// Create a program from two shaders
GLuint Context::createProgram(const char *path_vert_shader, const char *path_frag_shader)
{
    GLuint vertexShader = loadCompileShader(path_vert_shader, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadCompileShader(path_frag_shader, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // Flag the shaders for deletion
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Link and use the program
    glLinkProgram(shaderProgram);

    GLint isLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, (int *)&isLinked);

    if (isLinked == GL_FALSE)
    {
        char link_log[512];
        glGetShaderInfoLog(shaderProgram, 512, NULL, link_log);
        cout << "Program linking ERROR" << link_log << endl;
        exit(1);
    }
    else
    {
        cout << "Linking OK" << endl;
    }

    return shaderProgram;
}

void Context::initPrograms()
{
    //agents
    Program &ap = agentProgram;
    ap.id = createProgram("src/agent.vs", "src/agent.fs");
    ap.attr["position"] = glGetAttribLocation(ap.id, "position");
    ap.attr["direction"] = glGetAttribLocation(ap.id, "direction");
    ap.attr["velocity"] = glGetAttribLocation(ap.id, "velocity");
    ap.unif["view"] = glGetUniformLocation(ap.id, "view");
    ap.unif["projection"] = glGetUniformLocation(ap.id, "projection");
}

void Context::initBuffers()
{
    glGenVertexArrays(1, &(vao));
    glBindVertexArray(vao);
    glGenBuffers(1, &(posBuffer));
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, agents->size * sizeof(glm::vec3), nullptr, GL_STREAM_DRAW);

    glUseProgram(agentProgram.id);
    glVertexAttribPointer(agentProgram.attr["position"], 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(agentProgram.attr["position"]);

    glGenBuffers(1, &(dirBuffer));
    glBindBuffer(GL_ARRAY_BUFFER, dirBuffer);
    glBufferData(GL_ARRAY_BUFFER, agents->size * sizeof(glm::vec3), nullptr, GL_STREAM_DRAW);

    glUseProgram(agentProgram.id);
    glVertexAttribPointer(agentProgram.attr["direction"], 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(agentProgram.attr["direction"]);

    glGenBuffers(1, &(velBuffer));
    glBindBuffer(GL_ARRAY_BUFFER, velBuffer);
    glBufferData(GL_ARRAY_BUFFER, agents->size * sizeof(float), nullptr, GL_STREAM_DRAW);

    glUseProgram(agentProgram.id);
    glVertexAttribPointer(agentProgram.attr["velocity"], 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(agentProgram.attr["velocity"]);

    cout << "Initialization: " << glGetError() << endl;
}

void Context::setupBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, agents->size * sizeof(glm::vec3), agents->positions);

    glBindBuffer(GL_ARRAY_BUFFER, dirBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, agents->size * sizeof(glm::vec3), agents->directions);

    glBindBuffer(GL_ARRAY_BUFFER, velBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, agents->size * sizeof(float), agents->velocities);

    cout << "Setting: " << glGetError() << endl;
}

void Context::drawAgents()
{
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, agents->size * 4);

    //cout << "Drawing: " << glGetError() << endl;
}

//-----------------------------------------------------

Context::Context(AgentContainer *_agents)
    : camera(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0))
{
    agents = _agents;
    initPrograms();
    initBuffers();
    glClearColor(0.1, 0.1, 0.1, 1);
}

void Context::setup()
{
    setupBuffers();
}

void Context::draw()
{
    camera.frame();
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(agentProgram.id);
    glUniformMatrix4fv(agentProgram.unif["view"], 1, GL_FALSE, glm::value_ptr(camera.view));
    glUniformMatrix4fv(agentProgram.unif["projection"], 1, GL_FALSE, glm::value_ptr(camera.projection));

    drawAgents();
}

Context::~Context()
{
    glDeleteBuffers(1, &(posBuffer));
    //glDeleteBuffers(1, &(dirBuffer));
    //glDeleteBuffers(1, &(velBuffer));
}