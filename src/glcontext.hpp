#pragma once
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include "boid.hpp"
#include "box.hpp"
#include "environment.hpp"
#include "tree.hpp"
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
    Context(BoidContainer *agents, Environment * env);
    ~Context();

    void setupBox(const Box & bbox);
    void setupTree(OctalTreeContainer *tree);
    void setupGrid(const Box & bbox);

    void setupBoids();
    void setupTree();

    void computeShaderUpdateBoids();
    void computeShaderNaiveFlock();
    //void computeShaderTreeFlock();
    void computeShaderSortBoids();
    void computeShaderGridFlock();
    void copyBoidsToCPU();
    void draw();

    //subclasses
    Camera camera;

private:
    void loadShader(const char *path, string &contents);
    GLuint loadCompileShader(const char *path, GLenum shaderType);
    GLuint createProgram(const char *path_vert_shader, const char *path_frag_shader);
    GLuint createProgram(const char *path_compute_shader);
    string glslHeader();
    void initPrograms();
    void initBuffers();
    void drawBoxes();
    void drawBoids();

    //agent
    GLuint vao;
    GLuint bufAgents;
    GLuint bufGeometry;
    GLuint ebufGeometry;
    
    //boxes
    GLuint vaoBox;
    GLuint bufTree;
    GLuint bufBoxGeometry;
    GLuint ebufBoxGeometry;

    //rendering constants
    float boidSize;
    int32_t treeMemoryLimit;
    int32_t treeNodeCount;
    bool darkMode;

    //compute shader settings
    //GLuint vaoUpdating;
    //GLuint vaoFlockingNaive;
    //GLuint vaoFlockingTree;
    //GLuint vaoFlockingGrid;
    //GLuint vaoSortGrid;
    GLuint reductionBuffer;
    GLuint gridIndicesBuffer;
    int32_t invocations, groups;

    //programs
    Program agentProgram;
    Program boxProgram;
    Program updateBoidsProgram;
    Program reduceProgram;
    Program naiveFlockProgram;
    Program treeFlockProgram;
    Program sortProgram;
    Program gridReindexProgram;
    Program gridFlockProgram;

    //members
    BoidContainer * agents;
    OctalTreeContainer * tree;
    Environment * env;
};
