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

/**
 * @brief Structure representing OpenGL program
 * 
 * Also stores the location of attributes and uniforms and maps
 */
struct Program
{
    /**
     * @brief Id of the program
     */
    GLuint id;

    /**
     * @brief Map of attribute locations
     */
    unordered_map<string, GLuint> attr;

    /**
     * @brief Map of uniform locations
     */
    unordered_map<string, GLuint> unif;
};

/**
 * @brief Graphic context
 * 
 * Handles all OpenGL calls, dispatching compute shaders and managing resources
 */
class Context
{
public:
    /**
     * @brief Construct a new graphic context object
     * 
     * @param agents Pointer to boid container
     * @param env Pointer to environment with user settings
     */
    Context(BoidContainer *agents, Environment * env);

    /**
     * @brief Destroy the graphic context object
     */
    ~Context();

    /**
     * @brief Setup box shaders
     * 
     * Sets up box shaders and corresponding uniforms.
     * 
     * @param bbox Bounding box to be rendered
     */
    void setupBox(const Box & bbox);

    /**
     * @brief Setup box shaders to draw tree
     * 
     * Sets up box shaders (for the tree nodes) and corresponding uniforms.
     * 
     * @param tree octree to be rendered
     */
    void setupTree(OctalTreeContainer *tree);

    /**
     * @brief Setup box shaders and compute shaders
     * 
     * Sets up box shaders and corresponding uniforms,
     * also uniforms of additional compute shaders are set up. 
     * 
     * @param bbox Bounding box to be rendered
     */
    void setupGrid(const Box & bbox);

    /**
     * @brief Copy boids from CPU to GPU
     */
    void setupBoids();

    /**
     * @brief Copy octree from CPU to GPU
     */
    void setupTree();

    /**
     * @brief Dispatch compute shader for updating boids on GPU
     * 
     * Update compute shader applies the acceleration of boids and updates each boid,
     * moreover, it utilizes paralel reduction to obtain maximum and minimum number 
     * of boids in range.
     * 
     * In case the grid resolution is non zero, the shader calculates 
     * the index of cell containing the processed boid and stores the index in boid's ID
     * 
     * Paralel reduction uses shared memory.
     */
    void computeShaderUpdateBoids();

    /**
     * @brief Dispatches compute shader for naive flocking on GPU
     * 
     * Flocking in parallel, each boid checks all other boids. 
     * Uses only global memory, each boid iterates over boids starting 
     * at the next following boid and then the iteration loops around.
     * 
     */
    void computeShaderNaiveFlock();
    //void computeShaderTreeFlock();

    /**
     * @brief Dispatches compute shader for sorting according to boid's ID
     * 
     * Sorts boids utilizing bitonic merge sort. Utilizes shared memory 
     * in case more than one iteration is performed by the shader 
     * (when independent segments are spaller than size of the work group).
     * 
     * Also, a buffer with start indicies for each cell is filled.
     */
    void computeShaderSortBoids();

    /**
     * @brief Dispatches compute shader for grid-based flocking on GPU
     * 
     * Flocking in paralel, each boid checks surrounding cells and the containing cell.
     * The most effective flocking implementation.
     */
    void computeShaderGridFlock();

    /**
     * @brief Copy boids from GPU to CPU
     */
    void copyBoidsToCPU();

    /**
     * @brief Redraw the screen
     * 
     * Uses instancing to spead up the drawing of boids and boxes.
     */
    void draw(bool run);

    /**
     * @brief Camera class
     */
    Camera camera;


    bool capture;
private:
    /**
     * @brief Loads shader file from path
     * 
     * @param path Shader path
     * @param contents String filled by the contents of the shader 
     */
    void loadShader(const char *path, string &contents);

    /**
     * @brief Compiles OpenGL shader
     * 
     * @param path Path of the shader
     * @param shaderType Shader type
     * @return GLuint ID of the shader
     */
    GLuint loadCompileShader(const char *path, GLenum shaderType);

    /**
     * @brief Create a Program object
     * 
     * Creates OpenGL program consisting of vertex and fragment shader
     * 
     * @param path_vert_shader Path to vertex shader file
     * @param path_frag_shader Path tofragment shader file
     * @return GLuint returns ID of the program
     */
    GLuint createProgram(const char *path_vert_shader, const char *path_frag_shader);

    /**
     * @brief Create a Program object
     * 
     * Creates OpenGL program consisting of compute shader
     * 
     * @param path_compute_shader Path to compute shader file
     * @return GLuint returns ID of the program
     */
    GLuint createProgram(const char *path_compute_shader);

    /**
     * @brief Generates header for OpenGL shader
     * 
     * The header contains version and defines based
     * on environment values.
     * 
     * @return string Shader header
     */
    string glslHeader();

    /**
     * @brief Method to initialize all programs
     */
    void initPrograms();

    /**
     * @brief Method to initialize all buffers
     */
    void initBuffers();

    /**
     * @brief Draw boxes
     */
    void drawBoxes();

    /**  @brief Draw boids */
    void drawBoids();

    /** @brief VAO for boids-related buffers */
    GLuint vao;

    /** @brief Buffer with boid array copied from CPU */
    GLuint bufAgents;

    /** @brief Buffer with boid geometry */
    GLuint bufGeometry;

    /** @brief Buffer with indicies for boid geometry */
    GLuint ebufGeometry;
    
    /** @brief VAO for box-related buffers */
    GLuint vaoBox;
   
    /** @brief Buffer boxes array copied from CPU */
    GLuint bufTree;

    /** @brief Buffer with box geometry */
    GLuint bufBoxGeometry;
    
    /** @brief Buffer with box geometry indicies */
    GLuint ebufBoxGeometry;

    /** @brief Memory limit, number of tree nodes which fit into the allocated GPU buffer */
    int32_t treeMemoryLimit;

    /** @brief Number of tree nodes */
    int32_t treeNodeCount;
    
    /** @brief Size of boid object */
    float boidSize;
    
    /** @brief Flag indicating which color scheme to use */
    bool darkMode;

    /** @brief Buffer with min and max counts of boids in range */
    GLuint reductionBuffer;

    /** @brief Buffer with grid cell boid indicies */
    GLuint gridIndicesBuffer;

    /** @brief number of invocations and groups */
    int32_t invocations, groups;

    /** @brief Program for rendering boids */
    Program agentProgram;
    /** @brief Program for rendering boxes */
    Program boxProgram;
    /** @brief Program with compute shader for boid updates */
    Program updateBoidsProgram;
    /** @brief Program with compute shader for calculating min and max number of boids in range */
    Program reduceProgram;
    /** @brief Program with compute shader for naive GPU flocking */
    Program naiveFlockProgram;
    
    //Program treeFlockProgram;
    
    /** @brief Program with compute shader for bitonic mergesort */
    Program sortProgram;
    /** @brief Program with compute shader for reindexing cells */
    Program gridReindexProgram;
    /** @brief Program with compute shader for grid-based flocking */
    Program gridFlockProgram;

    /** @brief Container with boids */
    BoidContainer * agents;
    /** @brief Container with octree */
    OctalTreeContainer * tree;
    /** @brief Container user setting environment */
    Environment * env;
};
