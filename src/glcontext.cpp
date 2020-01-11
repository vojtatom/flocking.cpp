#define _USE_MATH_DEFINES //to use pi constants
#include <cmath>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

#include "glcontext.hpp"
#include "engine.hpp"
#include "environment.hpp"
#include "boid.hpp"
#include "camera.hpp"
#include "box.hpp"
#include "tree.hpp"

using namespace std;



void getGLError(const char * desc)
{
    GLenum error = glGetError();

    if (error != 0)
    {
        cout << desc << " " << error << endl;
        cout << gluErrorString(error) << endl;
    }
}

void Context::loadShader(const char *path, string &contents)
{
    cout << "loading " << path << endl;
    ifstream file(env->getPath() + string(path));
    stringstream buffer;

    buffer << file.rdbuf();
    contents = buffer.str();
}

string Context::glslHeader()
{
    float range = env->getFloat("flockingZone");
    float speedFac = env->getFloat("speedFactor");
    float forceLim = env->getFloat("forceLimit");

    string header = 
        "#version 430 core"
        "\n#define RANGE " + to_string(range) +
        "\n#define SPFAC " + to_string(speedFac) +
        "\n#define FORLIM " + to_string(forceLim) +
        "\n\n";

   return header;
}

GLuint Context::loadCompileShader(const char *path, GLenum shaderType)
{
    string contents;
    loadShader(path, contents);

    if (shaderType == GL_COMPUTE_SHADER)
    {
        string header = glslHeader();
        contents = header + contents;
    }

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
        glGetProgramInfoLog(shaderProgram, 512, NULL, link_log);
        cout << "Program linking ERROR: " << link_log << endl;
        exit(1);
    }
    else
    {
        cout << "Linking OK" << endl;
    }

    return shaderProgram;
}

GLuint Context::createProgram(const char *path_compute_shader)
{
    GLuint compute = loadCompileShader(path_compute_shader, GL_COMPUTE_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, compute);
    glLinkProgram(program);

    // Flag the shaders for deletion
    glDeleteShader(compute);

    // Link and use the program
    glLinkProgram(program);

    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);

    if (isLinked == GL_FALSE)
    {
        char link_log[512];
        glGetProgramInfoLog(program, 512, NULL, link_log);
        cout << "Program linking ERROR" << link_log << endl;
        exit(1);
    }
    else
    {
        cout << "Linking OK" << endl;
    }

    return program;
}

void Context::initPrograms()
{
    //agents
    Program &ap = agentProgram;
    ap.id = createProgram("src/shaders/agent.vs", "src/shaders/agent.fs");
    ap.attr["position"] = glGetAttribLocation(ap.id, "position");
    ap.attr["velocity"] = glGetAttribLocation(ap.id, "velocity");
    ap.attr["vert"] = glGetAttribLocation(ap.id, "vert");
    ap.attr["colorValue"] = glGetAttribLocation(ap.id, "colorValue");
    ap.unif["view"] = glGetUniformLocation(ap.id, "view");
    ap.unif["projection"] = glGetUniformLocation(ap.id, "projection");
    ap.unif["maxColorValue"] = glGetUniformLocation(ap.id, "maxColorValue");
    ap.unif["minColorValue"] = glGetUniformLocation(ap.id, "minColorValue");

    //boxes
    Program &bp = boxProgram;
    bp.id = createProgram("src/shaders/box.vs", "src/shaders/box.fs");
    bp.attr["vert"] = glGetAttribLocation(bp.id, "vert");
    bp.attr["low"] = glGetAttribLocation(bp.id, "low");
    bp.attr["high"] = glGetAttribLocation(bp.id, "high");
    bp.unif["view"] = glGetUniformLocation(bp.id, "view");
    bp.unif["projection"] = glGetUniformLocation(bp.id, "projection");
    bp.unif["darkMode"] = glGetUniformLocation(bp.id, "darkMode");

    //update boids
    Program &ub = updateBoidsProgram;
    ub.id = createProgram("src/shaders/update.glsl");
    ub.unif["size"] = glGetUniformLocation(ub.id, "size");
    ub.unif["low"] = glGetUniformLocation(ub.id, "low");
    ub.unif["diagonal"] = glGetUniformLocation(ub.id, "diagonal");
    ub.unif["gridRes"] = glGetUniformLocation(ub.id, "gridRes");

    //reduce
    Program &rb = reduceProgram;
    rb.id = createProgram("src/shaders/reduce.glsl");
    rb.unif["numGroups"] = glGetUniformLocation(rb.id, "numGroups");

    //naive flocking boids
    Program &nf = naiveFlockProgram;
    nf.id = createProgram("src/shaders/flockNaive.glsl");
    nf.unif["size"] = glGetUniformLocation(nf.id, "size");

    //tree flocking boids
    Program &tf = treeFlockProgram;
    tf.id = createProgram("src/shaders/flockTree.glsl");
    tf.unif["size"] = glGetUniformLocation(tf.id, "size");

    //grid flocking boids
    Program &sp = sortProgram;
    sp.id = createProgram("src/shaders/sortGrid.glsl");
    sp.unif["size"] = glGetUniformLocation(sp.id, "size");
    sp.unif["segmentSize"] = glGetUniformLocation(sp.id, "segmentSize");
    sp.unif["iteration"] = glGetUniformLocation(sp.id, "iteration");

    Program &ri = gridReindexProgram;
    ri.id = createProgram("src/shaders/reindexGrid.glsl");
    ri.unif["size"] = glGetUniformLocation(ri.id, "size");
    ri.unif["gridRes"] = glGetUniformLocation(ri.id, "gridRes");

    Program &fg = gridFlockProgram;
    fg.id = createProgram("src/shaders/flockGrid.glsl");
    fg.unif["size"] = glGetUniformLocation(fg.id, "size");
    fg.unif["gridRes"] = glGetUniformLocation(fg.id, "gridRes");
}

#define BASE 4 // number of primitives per boid
#define VERTCOUNT (BASE + 1) * 3
#define IDXCOUNT BASE * 3
#define BOXVERTSIZE 8 * 3
#define BOXELEMSIZE 24

void Context::initBuffers()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //GEOMETRY SETUP
    float verts[VERTCOUNT];

    for (size_t i = 0; i < BASE; i++)
    {
        verts[i * 3] = 0,                                             //x
            verts[i * 3 + 1] = sin(i * 2 * M_PI / (BASE)) * boidSize, //y
            verts[i * 3 + 2] = cos(i * 2 * M_PI / (BASE)) * boidSize; //z
    }

    verts[BASE * 3] = 2 * boidSize,
                 verts[BASE * 3 + 1] = 0,
                 verts[BASE * 3 + 2] = 0;

    unsigned int indices[IDXCOUNT];
    for (size_t i = 0; i < BASE; i++)
    {
        indices[i * 3] = i,
                    indices[i * 3 + 1] = (i + 1) % BASE,
                    indices[i * 3 + 2] = BASE;
    }

    //vertices
    glGenBuffers(1, &bufGeometry);
    glBindBuffer(GL_ARRAY_BUFFER, bufGeometry);
    glBufferData(GL_ARRAY_BUFFER, VERTCOUNT * sizeof(float), verts, GL_STATIC_DRAW);

    glUseProgram(agentProgram.id);
    glVertexAttribPointer(agentProgram.attr["vert"], 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(agentProgram.attr["vert"]);

    //indices
    glGenBuffers(1, &ebufGeometry);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebufGeometry);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IDXCOUNT * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    //AGENT BUFFERS SETUP
    glGenBuffers(1, &bufAgents);
    glBindBuffer(GL_ARRAY_BUFFER, bufAgents);
    glBufferData(GL_ARRAY_BUFFER, agents->size * sizeof(Boid), nullptr, GL_DYNAMIC_COPY);

    glVertexAttribPointer(agentProgram.attr["position"], 3, GL_FLOAT, GL_FALSE, sizeof(Boid), 0);
    glEnableVertexAttribArray(agentProgram.attr["position"]);
    glVertexAttribDivisor(agentProgram.attr["position"], 1);

    glVertexAttribPointer(agentProgram.attr["velocity"], 3, GL_FLOAT, GL_FALSE, sizeof(Boid), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(agentProgram.attr["velocity"]);
    glVertexAttribDivisor(agentProgram.attr["velocity"], 1);

    glVertexAttribPointer(agentProgram.attr["colorValue"], 1, GL_FLOAT, GL_FALSE, sizeof(Boid), (void *)(9 * sizeof(float) + sizeof(uint32_t)));
    glEnableVertexAttribArray(agentProgram.attr["colorValue"]);
    glVertexAttribDivisor(agentProgram.attr["colorValue"], 1);

    //setup SSBU for updating
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufAgents);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufAgents); // buffer on binding 0

    glGenBuffers(1, &reductionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, reductionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * groups * sizeof(float), nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, reductionBuffer); // buffer on binding 1

    glGenBuffers(1, &gridIndicesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridIndicesBuffer);
    glm::uvec3 gridRes = env->getVec("grid");
    uint32_t gridSize = gridRes.x * gridRes.y * gridRes.z;
    glBufferData(GL_SHADER_STORAGE_BUFFER, (1 + gridSize) * sizeof(uint32_t), nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, gridIndicesBuffer); // buffer on binding 1

    //BOX BUFFERS SETUP
    float boxvert[BOXVERTSIZE] = {
        0, 0, 0,
        1, 0, 0,
        1, 1, 0,
        0, 1, 0,
        0, 0, 1,
        1, 0, 1,
        1, 1, 1,
        0, 1, 1};

    uint32_t boxelem[BOXELEMSIZE] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        0, 4,
        1, 5,
        2, 6,
        3, 7,
        4, 5,
        5, 6,
        6, 7,
        7, 4};

    glGenVertexArrays(1, &vaoBox);
    glBindVertexArray(vaoBox);

    glGenBuffers(1, &bufBoxGeometry);
    glBindBuffer(GL_ARRAY_BUFFER, bufBoxGeometry);
    glBufferData(GL_ARRAY_BUFFER, BOXVERTSIZE * sizeof(float), boxvert, GL_STATIC_DRAW);

    glUseProgram(boxProgram.id);
    glVertexAttribPointer(boxProgram.attr["vert"], 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(boxProgram.attr["vert"]);

    glGenBuffers(1, &ebufBoxGeometry);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebufBoxGeometry);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, BOXELEMSIZE * sizeof(uint32_t), boxelem, GL_STATIC_DRAW);

    glGenBuffers(1, &bufTree);
    glBindBuffer(GL_ARRAY_BUFFER, bufTree);
    glBufferData(GL_ARRAY_BUFFER, treeMemoryLimit * sizeof(OctalTree), nullptr, GL_STREAM_DRAW);

    glVertexAttribPointer(boxProgram.attr["low"], 3, GL_FLOAT, GL_FALSE, sizeof(OctalTree), 0);
    glEnableVertexAttribArray(boxProgram.attr["low"]);
    glVertexAttribDivisor(boxProgram.attr["low"], 1);

    glVertexAttribPointer(boxProgram.attr["high"], 3, GL_FLOAT, GL_FALSE, sizeof(OctalTree), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(boxProgram.attr["high"]);
    glVertexAttribDivisor(boxProgram.attr["high"], 1);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufTree);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufTree); // buffer on binding 0
}

void Context::drawBoxes()
{
    glBindVertexArray(vaoBox);
    glUseProgram(boxProgram.id);
    glUniformMatrix4fv(boxProgram.unif["view"], 1, GL_FALSE, glm::value_ptr(camera.view));
    glUniformMatrix4fv(boxProgram.unif["projection"], 1, GL_FALSE, glm::value_ptr(camera.projection));
    glUniform1i(boxProgram.unif["darkMode"], darkMode);

    glDrawElementsInstanced(GL_LINES, BOXELEMSIZE, GL_UNSIGNED_INT, (void *)0, min(treeMemoryLimit, treeNodeCount));
}

void Context::drawBoids()
{
    glBindVertexArray(vao);

    /*Boid *boids = (Boid *)glMapNamedBufferRange(bufAgents, 0, agents->size * sizeof(Boid), GL_MAP_READ_BIT);
    glm::uvec3 gridRes = env->getVec("grid");
    for (size_t i = agents->size / 2; i < agents->size / 2 + 1; i++){
        unsigned int id = boids[i].id;
        unsigned int gridXY = gridRes.x * gridRes.y;
        unsigned int z = id / gridXY;
        unsigned int znext = (z + 1 + gridRes.z) % gridRes.z;
        unsigned int zprev = (z - 1 + gridRes.z) % gridRes.z;
        unsigned int tmpy = id - (z * gridXY);
        unsigned int y = tmpy / gridRes.x;
        unsigned int ynext = (y + 1 + gridRes.y) % gridRes.y;
        unsigned int yprev = (y - 1 + gridRes.y) % gridRes.y;
        unsigned int x = tmpy - y * gridRes.x;
        unsigned int xnext = (x + 1 + gridRes.x) % gridRes.x;
        unsigned int xprev = (x - 1 + gridRes.x) % gridRes.x;
        
        cout << "prev" << boids[i - 1].position << " " << boids[i - 1].id << endl;
        cout << i << endl;
        cout << boids[i].id << " " << boids[i].position << endl;
        cout << x << " " << y << " " << " " << z << endl;
        cout << xprev << " " << yprev << " " << " " << zprev << endl;
        cout << xnext << " " << ynext << " " << " " << znext << endl;



    }
    cout << endl;
    glUnmapNamedBuffer(bufAgents);*/

    //bind uniforms
    glUseProgram(agentProgram.id);
    glUniformMatrix4fv(agentProgram.unif["view"], 1, GL_FALSE, glm::value_ptr(camera.view));
    glUniformMatrix4fv(agentProgram.unif["projection"], 1, GL_FALSE, glm::value_ptr(camera.projection));
    glUniform1f(agentProgram.unif["maxColorValue"], agents->boidMaxCount);
    glUniform1f(agentProgram.unif["minColorValue"], agents->boidMinCount);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebufGeometry);
    glDrawElementsInstanced(GL_TRIANGLES, IDXCOUNT, GL_UNSIGNED_INT, (void *)0, agents->size);

    getGLError("drawing boids");
}

//-----------------------------------------------------

Context::Context(BoidContainer *_agents, Environment *_env)
    : camera(glm::vec3(0, 0, 100), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0))
{
    agents = _agents;
    tree = nullptr;
    env = _env;

    boidSize = env->getFloat("boidSize");
    treeMemoryLimit = env->getInt("treeMemoryLimit");
    darkMode = env->getInt("dark");

    int32_t gsx, gsy, gsz, gcx, gcy, gcz, inv;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &gsx);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &gsy);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &gsz);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &gcx);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &gcy);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &gcz);
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &inv);

    cout << "OPENGL INFO" << endl;
    cout << "Max work group size  " << gsx << " " << gsy << " " << gsz << endl;
    cout << "Max work group count " << gcx << " " << gcy << " " << gcz << endl;
    cout << "Max invocations      " << inv << endl;

    int32_t boidCount = 1 << env->getInt("boidCount");

    invocations = 1024; // play it safe with minimum allowed number
    groups = max(boidCount / invocations, 1);

    cout << "Using " << groups << " x " << invocations << " invocations" << endl;
    cout << "Boids " << boidCount << endl;

    initPrograms();
    initBuffers();

    if (darkMode)
        glClearColor(0.1, 0.1, 0.1, 1);
    else
        glClearColor(1, 1, 1, 1);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
}

Context::~Context()
{
    glDeleteBuffers(1, &(bufAgents));
    glDeleteBuffers(1, &(bufGeometry));
}

//-----------------------------------------------------
// Setup functions
//-----------------------------------------------------

//used with native GPU and CPU implementaitons
void Context::setupBox(const Box &bbox)
{
    //init tmp tree to allow for single rendering
    OctalTree tree;
    tree.bbox = bbox;

    glBindVertexArray(vaoBox);
    glBindBuffer(GL_ARRAY_BUFFER, bufTree);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(OctalTree), &tree);
    treeNodeCount = 1;

    glUseProgram(reduceProgram.id);
    glUniform1ui(reduceProgram.unif["numGroups"], groups);

    //setup for compute shader uniforms
    glUseProgram(updateBoidsProgram.id);
    glUniform3fv(updateBoidsProgram.unif["low"], 1, glm::value_ptr(bbox.low));
    glUniform3fv(updateBoidsProgram.unif["diagonal"], 1, glm::value_ptr(bbox.high - bbox.low));
    glUniform3uiv(updateBoidsProgram.unif["gridRes"], 1, glm::value_ptr(glm::uvec3(0)));

    getGLError("setup box");
}

void Context::setupTree(OctalTreeContainer *_tree)
{
    tree = _tree;

    //setup for compute shader uniforms
    glUseProgram(updateBoidsProgram.id);
    glUniform3fv(updateBoidsProgram.unif["low"], 1, glm::value_ptr(tree->getBoxLow()));
    glUniform3fv(updateBoidsProgram.unif["diagonal"], 1, glm::value_ptr(tree->getBoxDiagonal()));
    glUniform3uiv(updateBoidsProgram.unif["gridRes"], 1, glm::value_ptr(glm::uvec3(0)));

    glUseProgram(reduceProgram.id);
    glUniform1ui(reduceProgram.unif["numGroups"], groups);

    getGLError("setup tree");
}

void Context::setupGrid(const Box &bbox)
{
    //setup for compute shader uniforms
    setupBox(bbox);
    glm::uvec3 gridRes = env->getVec("grid");
    glUniform3uiv(updateBoidsProgram.unif["gridRes"], 1, glm::value_ptr(gridRes));

    glUseProgram(gridFlockProgram.id);
    glUniform3uiv(gridFlockProgram.unif["gridRes"], 1, glm::value_ptr(gridRes));

    glUseProgram(gridReindexProgram.id);
    glUniform3uiv(gridReindexProgram.unif["gridRes"], 1, glm::value_ptr(gridRes));

    getGLError("setup grid");
}

void Context::setupBoids()
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, bufAgents);
    glBufferSubData(GL_ARRAY_BUFFER, 0, agents->size * sizeof(Boid), agents->boids);
}

void Context::setupTree()
{
    if (tree == nullptr)
        return;

    OctalTree *trees = tree->getTrees(treeNodeCount); //fills treeNodeCount

    glBindVertexArray(vaoBox);
    glBindBuffer(GL_ARRAY_BUFFER, bufTree);
    glBufferSubData(GL_ARRAY_BUFFER, 0, min(treeMemoryLimit, treeNodeCount) * sizeof(OctalTree), trees);
}

//-----------------------------------------------------
// Compute shaders
//-----------------------------------------------------

void Context::computeShaderUpdateBoids()
{
    //bind uniforms
    glUseProgram(updateBoidsProgram.id);
    glUniform1i(updateBoidsProgram.unif["size"], agents->size);
    glDispatchCompute(groups, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    /*//COPY min max
    float *limits = (float *)glMapNamedBufferRange(reductionBuffer, 0, groups * 2 * sizeof(float), GL_MAP_READ_BIT);
    for (size_t i = 0; i < groups * 2; i++)
        cout << limits[i] << " " << flush;
    cout << endl;
    glUnmapNamedBuffer(reductionBuffer);*/

    glUseProgram(reduceProgram.id);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    //COPY min max
    float *limits = (float *)glMapNamedBufferRange(reductionBuffer, 0, 2 * sizeof(float), GL_MAP_READ_BIT);
    //cout << limits[0] << " " << limits[1] << endl;
    agents->updateStats(limits[0], limits[1]);
    //cout << agents->boidMaxCount << " " << limits[1] << endl;

    glUnmapNamedBuffer(reductionBuffer);

    getGLError("gpu update");
}

void Context::computeShaderNaiveFlock()
{
    //bind uniforms
    glUseProgram(naiveFlockProgram.id);
    glUniform1ui(naiveFlockProgram.unif["size"], agents->size);

    glDispatchCompute(groups, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    getGLError("gpu naive flocking");
}

/*void Context::computeShaderTreeFlock()
{
    //glBindVertexArray(vaoFlockingTree);
    
    glUseProgram(treeFlockProgram.id);
    glUniform1ui(treeFlockProgram.unif["size"], agents->size);

    glDispatchCompute(groups, 1, 1);
    //delayed sync until copy is neccesary

    cout << "GPU tree flocking: " << glGetError() << endl;
}*/

void Context::computeShaderSortBoids()
{
    //glBindVertexArray(vaoSortGrid);

    glUseProgram(sortProgram.id);
    glUniform1ui(sortProgram.unif["size"], agents->size);

    uint32_t segmentSize = 2;
    uint32_t iteration = 0;
    int32_t sizeOfIndependentBlock;

    do
    {
        //repeated dispatch only for sizes above number of invocations (needs sync)
        do
        {
            glUniform1ui(sortProgram.unif["segmentSize"], segmentSize);
            glUniform1ui(sortProgram.unif["iteration"], iteration);
            //cout << "sort " << iteration << " " << segmentSize << endl;
            glDispatchCompute(groups, 1, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            /*cout << "boids" << endl;
            int zeroes = 0;
            Boid * b = (Boid *) glMapNamedBufferRange(bufAgents, 0, agents->size * sizeof(Boid), GL_MAP_READ_BIT);
            for (size_t i = 0; i < agents->size; i++)
            {
                cout << b[i].id << " " << flush;
                if (b[i].id == 0)
                    zeroes++;
            }
            cout << endl;
            cout << zeroes << endl;
            glUnmapNamedBuffer(bufAgents);*/

            sizeOfIndependentBlock = segmentSize / int32_t(pow(2, iteration));
            iteration++;

        } while (sizeOfIndependentBlock > invocations);

        //increase step
        iteration = 0;
        segmentSize *= 2;
    } while (segmentSize <= agents->size);








    //update indices in buffer
    glUseProgram(gridReindexProgram.id);
    glUniform1ui(gridReindexProgram.unif["size"], agents->size);
    glDispatchCompute(groups, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);




    /*cout << "grid" << endl;
    glm::uvec3 gridRes = env->getVec("grid");
    uint32_t gridSize = gridRes.x * gridRes.y * gridRes.z;
    unsigned int * indices = (unsigned int *)glMapNamedBufferRange(gridIndicesBuffer, 0, (gridSize + 1) * sizeof(unsigned int ), GL_MAP_READ_BIT);
    for (size_t i = 0; i < gridSize + 1; i++)
        cout << indices[i] << " " << flush;
    cout << endl;
    glUnmapNamedBuffer(gridIndicesBuffer);*/




    getGLError("gpu sort");
    //exit(0);
}

void Context::computeShaderGridFlock()
{
    glUseProgram(gridFlockProgram.id);
    glUniform1ui(gridFlockProgram.unif["size"], agents->size);

    glDispatchCompute(groups, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    getGLError("gpu grid flocking");
}

//-----------------------------------------------------
// Utility functions
//-----------------------------------------------------

void Context::copyBoidsToCPU()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufAgents);

    //delayed sync
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    Boid *boids = (Boid *)glMapNamedBufferRange(bufAgents, 0, agents->size * sizeof(Boid), GL_MAP_READ_BIT);
    memcpy((void *)agents->boids, (const void *)boids, agents->size * sizeof(Boid));

    glUnmapNamedBuffer(bufAgents);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Context::draw()
{
    camera.frame();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    drawBoids();
    glEnable(GL_BLEND);
    drawBoxes();
    glBindVertexArray(vao);

    //cout << "\rBoxes: " << treeNodeCount << " Drawing: " << glGetError() << " Agents: " << agents->size << flush;
}