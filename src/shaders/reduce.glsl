//#version 430 core
//#define RANGE 40

layout (local_size_x = 1024) in;

shared float minimum[1024];
shared float maximum[1024];

layout(std430, binding = 1) buffer reductionBuffer
{
    float counts[];
};

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

//number of records in counts buffer / 2
layout(location = 1) uniform uint numGroups;

void main(void) 
{
    //uint id = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;
    //uint groupId = gl_WorkGroupID.x;
    uint localId = gl_LocalInvocationID.x;

    minimum[localId] = 3.402823e+15f;
    maximum[localId] = 0;


    if (localId < numGroups)
    {
        //copy to shared memory in the first work group
        minimum[localId] = counts[localId * 2];
        maximum[localId] = counts[localId * 2 + 1];
        
        //reduce
        reduce(localId, numGroups);

        //write by the first invocation into buffer
        if (localId == 0)
        {
            counts[0] = minimum[0];
            counts[1] = maximum[0];
        }
    }

}