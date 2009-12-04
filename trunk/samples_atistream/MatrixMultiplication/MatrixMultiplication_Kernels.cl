/*
 *  Calculates the naive matrix multiplication using the local memory 
 *  loads the blocks along the common dimension to local memories
 *  performs naive matrix multiplication on the blocks that are loaded
 *  
 */

__kernel void 
matrixMultiplication(__global float * output,
                     __global float * input0,
                     __global float * input1,
                     __local  float * local0,
                     __local  float * local1,
                     const    uint    width0,
                     const    uint    width1,
                     const    uint    blockSize)
{
    /* get the block ids in both the directions */
    int bx = get_group_id(0);
    int by = get_group_id(1);
   
    /* get the local ids within the block */
    int tx = get_local_id(0); 
    int ty = get_local_id(1);

    /* 
     * width0 is the common dimension between both the matrices 
     * calculating number of blocks along the common dimension 
     */
    int n = width0/blockSize;
    
    /* initialize accumulator */
    float acc = 0;
   
    /* For each block along the common dimension for both the matrices */
    for(int b= 0; b< n ; ++b)
    {
        /* Copy elements of block with blockIds(b,by) of input0 to local memory local0 */
        /* each thread writes only one element and waits for others to sync            */

        /* calculate global ids of the element */
        int globalIdx0 = b *blockSize + tx;
        int globalIdy0 = by*blockSize + ty;
        int globalId0  = globalIdy0*width0 + globalIdx0;

        /* copy input0 to the local0 */
        local0[ty*blockSize + tx] = input0[globalId0];

        /* Copy elements of block with blockIds(bx,b) of input1 to local memory local1 */
        /* each thread writes only one element and waits for others to sync            */

        /* calculate global ids of the element */
        int globalIdx1 = bx * blockSize + tx;
        int globalIdy1 = b  * blockSize + ty;
        int globalId1  = globalIdy1*width1 + globalIdx1;

        /* copy input1 to the local1 */
        local1[ty*blockSize + tx] = input1[globalId1];
   
        /* sync threads of the work group for the blocks to get copied to local memory */
        barrier(CLK_LOCAL_MEM_FENCE);
       
        /* perform naive matrix multiplication using local memory blocks */
        for(int k=0; k < blockSize; ++k)
        {
            acc += local0[ty*blockSize + k]*local1[k*blockSize + tx];
        }

        /* sync before the next set of blocks are copied into local memory */
        barrier(CLK_LOCAL_MEM_FENCE);
    }
   
    /* find the global location in output */
    int x  = get_global_id(0);
    int y  = get_global_id(1);
    int outIndex = y*width1 + x;
   
    /* write the accumulator to the output */
    output[outIndex] = acc;
}
