//#version 430 core
//#define SPFAC 2.0f
//#define FORLIM 0.1f

layout (local_size_x = 1024) in;

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

layout(std430, binding = 0) buffer boidBuffer
{
    Boid boids[];
};

layout(location = 1) uniform uint size;

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


void main(void) 
{
    uint id = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;

    uint otherid;
    uint inrange = 0;
    uint viewCount = 0;
    float angle;

    vec3 align = vec3(0.f);
    vec3 cohesion = vec3(0.f);
    vec3 separation = vec3(0.f);
    
    mat4 view = mat4(1.f);
    vec3 tmp = vec3(0.f);

    //valid boid
    if (id < size)
    {
        GLSLBoid me = boidToGLSLBoid(boids[id]);
        
        //go through all other boids
        for(uint i = 1; i < size; ++i)
        {
            otherid = (id + i) % size;
            //GLSLBoid other = boidToGLSLBoid(boids[otherid]);

            vec3 opos = vec3(boids[otherid].pos[0], boids[otherid].pos[1], boids[otherid].pos[2]);
            if (distance(me.pos, opos) < 40.0f)
            {
                inrange++;
                align += vec3(boids[otherid].vel[0], boids[otherid].vel[1], boids[otherid].vel[2]);
                cohesion += opos;
                separation += normalize(me.pos - opos);

                tmp = opos - me.pos;
                angle = acos(dot(normalize(me.vel), normalize(tmp)));

                if (angle < 1)
                {
                    view = view * rotationMatrix(cross(me.vel, tmp), angle * 0.01f);
                    viewCount++;
                }
            }
        }

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