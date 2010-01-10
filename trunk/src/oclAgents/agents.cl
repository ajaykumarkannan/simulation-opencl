/*
 * Agent based Simulation
 * Device File
 *
 *
 * 08 - Nov - 2009  VEC     init, not functional yet
 */


////////////////////////////////////////////////////////////////////////////////
//! Matrix multiplication on the device: C = A * B
//! WA is A's width and WB is B's width
////////////////////////////////////////////////////////////////////////////////
__kernel void
agents( __global agent_vector_t * fixed_agents, __global agent_vector_t * moving_agents, __global unsigned int * f_count, 
	    __global unsigned int * m_count, __global int * testdec)
{
    unsigned int i;
    
    for(i=0; i < *m_count; i++)
    {
        fixed_agents[i].pos_x = 1.0;
        
        moving_agents[i].pos_x = 100.000;
        moving_agents[i].pos_y = 200.000;
        moving_agents[i].mov_x = 300.000;
        moving_agents[i].mov_y = 400.000;
    }

    *testdec = 532;
}

