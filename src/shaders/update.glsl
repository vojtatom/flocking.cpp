//#version 430 core
//#define RANGE 40

layout (local_size_x = 1024) in;

shared float minimum[1024];
shared float maximum[1024];


struct Boid{
    float posx;
    float posy;
    float posz;
    float velx;
    float vely;
    float velz;
    float accx;
    float accy;
    float accz;
    uint   id;
    float countAround;
};

struct GLSLBoid{
    vec3 pos;
    vec3 vel;
    vec3 acc;
    uint   id;
    float countAround;
};

layout(std430, binding = 0) buffer boidBuffer
{
    Boid boids[];
};

layout(std430, binding = 1) buffer reductionBuffer
{
    float counts[];
};

//number of boids
layout(location = 1) uniform int size;

//dimensions of the space
layout(location = 2) uniform vec3 low;
layout(location = 3) uniform vec3 diagonal;

//respolution of the uniform grid
layout(location = 4) uniform uvec3 gridRes;

GLSLBoid boidToGLSLBoid(Boid boid)
{
    GLSLBoid newBoid;
    newBoid.pos = vec3(boid.posx, boid.posy, boid.posz);
    newBoid.vel = vec3(boid.velx, boid.vely, boid.velz);
    newBoid.acc = vec3(boid.accx, boid.accy, boid.accz);
    newBoid.id = boid.id;
    newBoid.countAround = boid.countAround;
    return newBoid;
}

void GLSLBoidToBuffer(GLSLBoid boid, uint id)
{
    boids[id].posx = boid.pos.x;
    boids[id].posy = boid.pos.y;
    boids[id].posz = boid.pos.z;
    boids[id].velx = boid.vel.x;
    boids[id].vely = boid.vel.y;
    boids[id].velz = boid.vel.z;
    boids[id].accx = boid.acc.x;
    boids[id].accy = boid.acc.y;
    boids[id].accz = boid.acc.z;
    boids[id].id = boid.id;
    boids[id].countAround = boid.countAround;
}

void reduce(uint localId, uint range)
{
    //paralel reduction to get minimum and maximum
    barrier();
    uint step = range / 2;
    while ( step != 0 ) {
        if (localId < step)
        {
            minimum[localId] = min(minimum[localId], minimum[localId + step]);
            maximum[localId] = max(maximum[localId], maximum[localId + step]);
        }
        barrier();
        step /= 2;
    }
}

void main(void) 
{
    uint id = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;
    uint groupId = gl_WorkGroupID.x;
    uint localId = gl_LocalInvocationID.x;

    minimum[localId] = 3.402823e+15f;
    maximum[localId] = 0;

    //valid boid
    if (id < size)
    {
        //get boid from the buffer
        GLSLBoid boid = boidToGLSLBoid(boids[id]);

        //update boid (apply forces)
        boid.pos = mod(((boid.pos + boid.vel) - low + diagonal), diagonal) + low;
        boid.vel += boid.acc;
        boid.acc = vec3(0.0f);

        //calculate position of the boid in the grid
        if (gridRes.x != 0)
        {
            vec3 range = diagonal / gridRes;
            uvec3 vgridIdx = uvec3((boid.pos - low) / range);
            boid.id = uint(gridRes.x * gridRes.y * vgridIdx.z + gridRes.x * vgridIdx.y + vgridIdx.x);
        }

        //save boid back to buffer
        GLSLBoidToBuffer(boid, id);

        //save to shared memory
        minimum[localId] = boid.countAround;
        maximum[localId] = boid.countAround;

        //perform paralel reduction
        reduce(localId, gl_WorkGroupSize.x);

        //write by the first invocation into buffer
        if (localId == 0)
        {
            counts[groupId * 2] = minimum[0];
            counts[groupId * 2 + 1] = maximum[0];
        }

        if (localId < gl_NumWorkGroups.x && groupId == 0)
        {
            //copy to shared memory in the first work group
            minimum[localId] = counts[localId * 2];
            maximum[localId] = counts[localId * 2 + 1];
            
            //reduce
            reduce(localId, gl_NumWorkGroups.x);

            //write by the first invocation into buffer
            if (localId == 0)
            {
                counts[0] = minimum[0];
                counts[1] = maximum[0];
            }
        }
    }
}