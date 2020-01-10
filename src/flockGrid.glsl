#version 430 core

layout (local_size_x = 1024) in;

#define RANGE 40
#define SUBTREES 8
#define BOIDSPERTREE 16
#define NONE -1

#define MAXTREETRAVERSED 20

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

//BUFFERS AND UNIFORMS
layout(std430, binding = 0) buffer boidBuffer
{
    Boid boids[];
};

layout(std430, binding = 3) buffer indexBuffer
{
    uint indices[];
};

layout(location = 1) uniform uint size;
layout(location = 2) uniform uvec3 gridRes;



//not really global since there is nothing like that in shaders...
uint inrange = 0;
uint viewCount = 0;
vec3 align = vec3(0.f);
vec3 cohesion = vec3(0.f);
vec3 separation = vec3(0.f);
mat4 view = mat4(1.f);
uint myId;
GLSLBoid me;

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

void flock(uint boidIdx)
{
    if (myId == boidIdx)
        return;

    vec3 otherPos = vec3(boids[boidIdx].pos[0], boids[boidIdx].pos[1], boids[boidIdx].pos[2]);
    
    if (distance(me.pos, otherPos) > RANGE)
        return;

    vec3 otherVel = vec3(boids[boidIdx].vel[0], boids[boidIdx].vel[1], boids[boidIdx].vel[2]);
    
    inrange++;
    align += otherVel;
    cohesion += otherPos;
    separation += normalize(me.pos - otherPos);

    vec3 tmp = otherPos - me.pos;
    float angle = acos(dot(normalize(me.vel), normalize(tmp)));

    if (angle < 1)
    {
        view = view * rotationMatrix(cross(me.vel, tmp), angle * 0.01f);
        viewCount++;
    }
}

void traverseCell(uint x, uint y, uint z)
{
    uint id = z * gridRes.x * gridRes.y + y * gridRes.x + x;
    uint start = indices[id];
    uint end = indices[id + 1];

    for (uint i = start; i < end; ++i)
        flock(i);
}


#define SPFAC 2.0f
#define FORLIM 0.1f

void main(void) 
{
    myId = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;


    //valid boid
    if (myId < size)
    {
        me = boidToGLSLBoid(boids[myId]);

        uint gridXY = gridRes.x * gridRes.y;
        uint z = me.id / gridXY;
        uint znext = (z + 1 + gridRes.z) % gridRes.z;
        uint zprev = (z - 1 + gridRes.z) % gridRes.z;
        uint tmpy = me.id - (z * gridXY);
        uint y = tmpy / gridRes.x;
        uint ynext = (y + 1 + gridRes.y) % gridRes.y;
        uint yprev = (y - 1 + gridRes.y) % gridRes.y;
        uint x = tmpy - y * gridRes.x;
        uint xnext = (x + 1 + gridRes.x) % gridRes.x;
        uint xprev = (x - 1 + gridRes.x) % gridRes.x;
        
        traverseCell(xprev, yprev, zprev);
        traverseCell(x, yprev, zprev);
        traverseCell(xnext, yprev, zprev);

        traverseCell(xprev, y, zprev);
        traverseCell(x, y, zprev);
        traverseCell(xnext, y, zprev);

        traverseCell(xprev, ynext, zprev);
        traverseCell(x, ynext, zprev);
        traverseCell(xnext, ynext, zprev);

        traverseCell(xprev, yprev, z);
        traverseCell(x, yprev, z);
        traverseCell(xnext, yprev, z);

        traverseCell(xprev, y, z);
        traverseCell(x, y, z);
        traverseCell(xnext, y, z);

        traverseCell(xprev, ynext, z);
        traverseCell(x, ynext, z);
        traverseCell(xnext, ynext, z);

        traverseCell(xprev, yprev, znext);
        traverseCell(x, yprev, znext);
        traverseCell(xnext, yprev, znext);

        traverseCell(xprev, y, znext);
        traverseCell(x, y, znext);
        traverseCell(xnext, y, znext);

        traverseCell(xprev, ynext, znext);
        traverseCell(x, ynext, znext);
        traverseCell(xnext, ynext, znext);

        if (inrange > 0)
        {
            me.acc += limit((normalize(align) * SPFAC - me.vel), FORLIM);
            me.acc += limit((normalize(cohesion / inrange - me.pos) * SPFAC - me.vel), FORLIM);
            me.acc += limit((normalize(separation) * SPFAC - me.vel), FORLIM);

            if (viewCount > 0)
            {
                vec3 tmp = vec3(view * vec4(me.vel, 1.0f));
                me.acc += limit(normalize(tmp) * SPFAC - me.vel, FORLIM);
            }
        }

        me.countAround = float(inrange);
        GLSLBoidToBuffer(me, myId);
    }

}