/* 
 * For a description of the algorithm and the terms used, please see the
 * documentation for this sample.
 *
 * Quasi Random Sequence
 * Output size : n_dimensions * n_vectors
 * Input size: n_dimensions * n_directions 
 * shared buffer size : n_directions
 * Number of blocks : n_dimensions
 * First, all the direction numbers for a dimension are cached into
 * shared memory. Then each thread writes a single vector value by
 * using all the direction numbers from the shared memory.
 *
 */

__kernel void QuasiRandomSequence(__global  float* output,
                                  __global  uint* input,
					    __local uint* shared)
{
	uint global_id = get_global_id(0);
	uint local_id = get_local_id(0);
	uint group_id = get_group_id(0);

	if( local_id < 32)
	{
		shared[local_id] = input[group_id * 32 + local_id];
	}

	barrier(CLK_LOCAL_MEM_FENCE);
	
	uint temp = 0;
	
	for(int k=0; k < 32; k++)
	{
		int mask = pow(2, k); 
		temp ^= ((local_id & mask)>>k) * shared[k];
	}
	
	if(global_id==0)
	{
		output[global_id] = 0;
	}
	else
	{
		output[global_id] = temp / pow(2, 32);
	}

}
