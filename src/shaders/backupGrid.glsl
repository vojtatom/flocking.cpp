//#version 430 core
//#define RANGE 40

layout (local_size_x = 512) in;


//STRUCTS
struct Boid{
    float pos[3];
    float vel[3];
    float acc[3];
    int   id;
    float countAround;
};

shared Boid lboids[1024];

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

void localSwap(uint elemID, uint oelemID)
{
    Boid tmp = lboids[elemID];
    lboids[elemID] = lboids[oelemID];
    lboids[oelemID] = tmp;
}

void main(void) 
{
    uint id = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;
    uint locId = gl_LocalInvocationID.x;
    uint hstep = segmentSize >> 1;

    //activate only if the invocation is in the first half of all invocations
    //consequently, it is not neccessary to launch the second half of invocations
    if (id < (size >> 1))
    {
        bool asc = hstep > (id % segmentSize);
        hstep = hstep >> iteration;
        //hstep /= uint(pow(2, iteration));
        uint elemID, oelemID;

        if (hstep >= gl_WorkGroupSize.x)
        {
            elemID = id + (id / hstep) * hstep;
            oelemID = elemID + hstep;
            bool comp = boids[elemID].id < boids[oelemID].id;
            
            //negate result if neccesary
            if (asc)
                comp = !comp;

            if (comp)
                swap(elemID, oelemID);

        } else  {

            //copy to local
            uint copyId = id * 2;
            uint locCopyId = locId * 2;
            lboids[locCopyId] = boids[copyId];
            lboids[locCopyId + 1] = boids[copyId + 1];
            barrier();
            //memoryBarrierShared();

            //elemID = id + (id / hstep) * hstep;
            //oelemID = elemID + hstep;
            elemID = locId + (locId / hstep) * hstep;
            oelemID = elemID + hstep;

            while(hstep > 0)
            {
                
                //bool comp = boids[elemID].id < boids[oelemID].id;
                bool comp = lboids[elemID].id < lboids[oelemID].id;
                
                //negate result if neccesary
                if (asc)
                    comp = !comp;
                //comp = (!asc && comp) || (asc && !comp);

                if (comp)
                    localSwap(elemID, oelemID);
                    //swap(elemID, oelemID);
                
                //iterate 
                hstep >>= 1;

                //elemID = id + (id / hstep) * hstep;
                //oelemID = elemID + hstep;
                elemID = locId + (locId / hstep) * hstep;
                oelemID = elemID + hstep;
                barrier();
                //memoryBarrierShared();
            }

            boids[copyId] = lboids[locCopyId];
            boids[copyId + 1] = lboids[locCopyId + 1];
        }


    }

}

//buggz main
void main(void) 
{
    uint id = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;
    //uint locId = gl_LocalInvocationID.x;
    uint hstep = segmentSize >> 1;

    //activate only if the invocation is in the first half of all invocations
    //consequently, it is not neccessary to launch the second half of invocations
    if (id < (size >> 1))
    {
        bool asc = hstep > (id % segmentSize);
        hstep = hstep >> iteration;
        //hstep /= uint(pow(2, iteration));
        uint elemID, oelemID;

        if (hstep > gl_WorkGroupSize.x)
        {
            elemID = id + (id / hstep) * hstep;
            oelemID = elemID + hstep;
            bool comp = boids[elemID].id < boids[oelemID].id;
            
            //negate result if neccesary
            if (asc)
                comp = !comp;

            if (comp)
                swap(elemID, oelemID);

        } else  {

            //copy to local
            //uint copyId = id * 2;
            //uint locCopyId = locId * 2;
            //lboids[locCopyId] = boids[copyId];
            //lboids[locCopyId + 1] = boids[copyId + 1];
            //barrier();

            elemID = id + (id / hstep) * hstep;
            oelemID = elemID + hstep;
            //elemID = locId + (locId / hstep) * hstep;
            //oelemID = elemID + hstep;

            while(hstep > 0)
            {
                
                bool comp = boids[elemID].id < boids[oelemID].id;
                //bool comp = lboids[elemID].id < lboids[oelemID].id;
                
                //negate result if neccesary
                if (asc)
                    comp = !comp;
                //comp = (!asc && comp) || (asc && !comp);

                if (comp)
                    swap(elemID, oelemID);
                    //localSwap(elemID, oelemID);
                
                //iterate 
                hstep >>= 1;

                elemID = id + (id / hstep) * hstep;
                oelemID = elemID + hstep;
                //elemID = locId + (locId / hstep) * hstep;
                //oelemID = elemID + hstep;
                barrier();
            }

            //boids[copyId] = lboids[locCopyId];
            //boids[copyId + 1] = lboids[locCopyId + 1];
        }


    }

}