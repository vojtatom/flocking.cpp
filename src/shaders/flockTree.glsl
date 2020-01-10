//#version 430 core
//#define RANGE 40
//#define SPFAC 2.0f
//#define FORLIM 0.1f
//
#define NONE -1
#define SUBTREES 8
#define BOIDSPERTREE 16
#define MAXTREETRAVERSED 20

layout (local_size_x = 1024) in;


//STRUCTS
struct Boid{
    float pos[3];
    float vel[3];
    float acc[3];
    int   id;
    float countAround;
};

struct GLSLBoid{
    vec3 pos;
    vec3 vel;
    vec3 acc;
    int   id;
    float countAround;
};

struct Octree{
    float low[3];
    float high[3];
    int subtree[SUBTREES];
    int boidIdx[BOIDSPERTREE];
    int filled;
};


//BUFFERS AND UNIFORMS
layout(std430, binding = 0) buffer boidBuffer
{
    Boid boids[];
};

layout(std430, binding = 2) buffer treeBuffer
{
    Octree trees[];
};

layout(location = 1) uniform uint size;


//FUNCTIONS
GLSLBoid boidToGLSLBoid(Boid boid)
{
    GLSLBoid newBoid;
    newBoid.pos = vec3(boid.pos[0], boid.pos[1], boid.pos[2]);
    newBoid.vel = vec3(boid.vel[0], boid.vel[1], boid.vel[2]);
    newBoid.acc = vec3(boid.acc[0], boid.acc[1], boid.acc[2]);
    newBoid.id = boid.id;
    newBoid.countAround = boid.countAround;
    return newBoid;
}

void GLSLBoidToBuffer(GLSLBoid boid, uint id)
{
    boids[id].pos[0] = boid.pos.x;
    boids[id].pos[1] = boid.pos.y;
    boids[id].pos[2] = boid.pos.z;
    boids[id].vel[0] = boid.vel.x;
    boids[id].vel[1] = boid.vel.y;
    boids[id].vel[2] = boid.vel.z;
    boids[id].acc[0] = boid.acc.x;
    boids[id].acc[1] = boid.acc.y;
    boids[id].acc[2] = boid.acc.z;
    boids[id].id = boid.id;
    boids[id].countAround = boid.countAround;
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

vec3 limit(vec3 v, float limit){
    if (dot(v, v) > limit * limit){
        return normalize(v) * limit;
    }
    return v;
}




uint otherid;
uint inrange = 0;
uint viewCount = 0;
float dist;
float angle;

vec3 align = vec3(0.f);
vec3 cohesion = vec3(0.f);
vec3 separation = vec3(0.f);

mat4 view = mat4(1.f);
vec3 tmp = vec3(0.f);
GLSLBoid me;


bool covered(vec3 point, int treeIdx)
{
    vec3 closest;
    closest.x = min(max(trees[treeIdx].low[0], point.x), trees[treeIdx].high[0]);
    closest.y = min(max(trees[treeIdx].low[1], point.y), trees[treeIdx].high[1]);
    closest.z = min(max(trees[treeIdx].low[2], point.z), trees[treeIdx].high[2]);
    return distance(point, closest) < RANGE;
}


void flock(int boidIdx, vec3 otherPos)
{
    inrange++;
    
    align += vec3(boids[boidIdx].vel[0], boids[boidIdx].vel[1], boids[boidIdx].vel[2]);
    cohesion += otherPos;
    separation += normalize(me.pos - otherPos);

    tmp = otherPos - me.pos;
    angle = acos(dot(normalize(me.vel), normalize(tmp)));

    if (angle < 1)
    {
        view = view * rotationMatrix(cross(me.vel, tmp), angle * 0.01f);
        viewCount++;
    }
}

void flockTreeBoids(vec3 point, int treeIdx)
{
    //manage subtrees
    for (int i = 0; i < trees[treeIdx].filled; i++)
    {
        int lidx = trees[treeIdx].boidIdx[i];
        if (lidx == me.id)
            continue;

        vec3 pos = vec3(boids[lidx].pos[0], boids[lidx].pos[1], boids[lidx].pos[2]);
        if (distance(pos, point) < RANGE)
            flock(lidx, pos);
    }
}


void traverse(vec3 point)
{
    int lidx, stackIdx;
    int treeStack[MAXTREETRAVERSED];
    int filled = 0;

    treeStack[0] = 0;
    stackIdx = 1;

    for(int i = 0; i < MAXTREETRAVERSED; ++i)
    {
        if (stackIdx == i)
            break;

        flockTreeBoids(point, treeStack[i]);

        for(int j = 0; j < SUBTREES; ++j)
        {
            lidx = trees[treeStack[i]].subtree[j];   
            if (lidx != NONE && stackIdx < MAXTREETRAVERSED && covered(point, lidx))
                treeStack[stackIdx++] = lidx;
        }
    }

    //memoryBarrier();
}


void main(void) 
{
    uint id = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;


    //valid boid
    if (id < size)
    {
        me = boidToGLSLBoid(boids[id]);
        
        //traverse the tree from the root
        traverse(me.pos);

        if (inrange > 0)
        {
            me.acc += limit((normalize(align / inrange) * SPFAC - me.vel), FORLIM);
            me.acc += limit((normalize(cohesion / inrange - me.pos) * SPFAC - me.vel), FORLIM);
            me.acc += limit((normalize(separation / inrange) * SPFAC - me.vel), FORLIM);

            if (viewCount > 0)
            {
                tmp = vec3(view * vec4(me.vel, 1.0f));
                me.acc += limit(normalize(tmp) * SPFAC - me.vel, FORLIM);
            }
        }

        me.countAround = inrange;
        GLSLBoidToBuffer(me, id);
    }

}