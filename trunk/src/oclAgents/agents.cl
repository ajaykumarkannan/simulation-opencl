/*
 * Agent based Simulation
 * Device File
 *
 *
 * 08 - Nov - 2009  VEC     init, not functional yet
 */

#define RADIUS 5 /* looking distance of moving agents */


////////////////////////////////////////////////////////////////////////////////
//! Matrix multiplication on the device: C = A * B
//! WA is A's width and WB is B's width
////////////////////////////////////////////////////////////////////////////////
__kernel void
agents( __global agent_vector_t * fixed_agents, __global agent_vector_t * moving_agents, __global unsigned int * f_count, 
	    __global unsigned int * m_count)
{
    unsigned int r2 = RADIUS * RADIUS;
    unsigned int m,f,i;
    
    moving_agents[0].pos_x = 100.000;
    moving_agents[0].pos_y = 200.000;
    moving_agents[0].mov_x = 300.000;
    moving_agents[0].mov_y = 400.000;
    
    
    for(m=0; m < *m_count; m++)
    {
        unsigned int in_radius_count = 0;
        //agent_vector_t * in_radius[*f_count];
        agent_vector_t * in_radius[AGENTS_FIXED_COUNT];

        /* check for every fixed agent if in radius */
        for(f=0; f < *f_count; f++)
        {        
            float f_x = fixed_agents[m].pos_x;
            float f_y = fixed_agents[m].pos_y;
        
            if ((f_x * f_x) + (f_y * f_y) <= r2)    /* check if in radius */
            { 
                *in_radius[in_radius_count] = fixed_agents[m];
                in_radius_count++;
            }
        }
        /* now we know all fixed agents in range, calculate new dir now */
        for(i=0; i < in_radius_count; i++)
        {
            moving_agents[m].mov_x += in_radius[i]->mov_x;
            moving_agents[m].mov_y += in_radius[i]->mov_y;
        }
        
        if(in_radius_count) /* only if there are some in radius */
        {
            moving_agents[m].mov_x = moving_agents[m].mov_x / (in_radius_count + 1);
            moving_agents[m].mov_y = moving_agents[m].mov_y / (in_radius_count + 1);
        }
        
        /* now move agent*/
        moving_agents[m].pos_x += moving_agents[m].mov_x; 
        moving_agents[m].pos_y += moving_agents[m].mov_y;

    }

    

}

