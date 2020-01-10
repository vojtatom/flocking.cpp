//#version 430 core
//#define RANGE 40

layout (local_size_x = 1024) in;


//STRUCTS
struct Boid{
    float pos[3];
    float vel[3];
    float acc[3];
    int   id;
    float countAround;
};

/*struct sBoid{
    uint   gridId;
    uint   boidId;
};*/


//shared sBoid lboids[1024];

//BUFFERS AND UNIFORMS
layout(std430, binding = 0) buffer boidBuffer
{
    Boid boids[];
};

//number of boids
layout(location = 1) uniform uint size;

//size of segment up or down sorted + iteration of sorting
layout(location = 4) uniform uint segmentSize;
layout(location = 5) uniform uint iteration;


void swap(uint elemID, uint oelemID)
{
    Boid tmp = boids[elemID];
    boids[elemID] = boids[oelemID];
    boids[oelemID] = tmp;
}

void main(void) 
{
    uint id = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;
    uint hstep = segmentSize >> 1;

    //activate only if the invocation is in the first half of all invocations
    //consequently, it is not neccessary to launch the second half of invocations
    if (id < size >> 1)
    {
        bool asc = hstep > (id % (segmentSize));
        hstep = hstep >> iteration;
        //hstep /= uint(pow(2, iteration));
        uint elemID = id + (id / hstep) * hstep;
        uint oelemID = elemID + hstep;

        if (hstep >= gl_WorkGroupSize.x)
        {
            bool comp = boids[elemID].id < boids[oelemID].id;
            
            //negate result if neccesary
            if (asc)
                comp = !comp;

            if (comp)
                swap(elemID, oelemID);

        } else  {
            while(hstep > 0)
            {
                
                bool comp = boids[elemID].id < boids[oelemID].id;
                
                //negate result if neccesary
                if (asc)
                    comp = !comp;
                //comp = (!asc && comp) || (asc && !comp);

                if (comp)
                    swap(elemID, oelemID);
                
                elemID = id + (id / hstep) * hstep;
                oelemID = elemID + hstep;

                //iterate 
                hstep >>= 1;
                barrier();
            }
        }


    }

}