/*
 * Agent based Simulation
 * Device File
 *
 *
 * 08 - Nov - 2009  VEC     init, not functional yet
 * 08 - Mar - 2010  VEC     fully functional, and tested
 */


//unsigned int gpu_run = 0;
////////////////////////////////////////////////////////////////////////////////
//! Matrix multiplication on the device: C = A * B
//! WA is A's width and WB is B's width
////////////////////////////////////////////////////////////////////////////////
__kernel void
agents( __global agent_vector_t * fixed_agents, __global agent_vector_t * moving_agents, __global unsigned int * f_count, 
	    __global unsigned int * m_count, __global int * ret)
{
    unsigned int r2 = LOOKAHEAD_RADIUS * LOOKAHEAD_RADIUS;
    unsigned int m,f,i;
    
    // get moving agent to work on
    m = get_global_id(0);

    unsigned int in_radius_count = 0;
    int new_dir_x = 0;
    int new_dir_y = 0;
    //agent_vector_t * in_radius[*f_count];
    agent_vector_t in_radius[AGENTS_FIXED_COUNT]; // improve performance with pointer traversal

    // check for every fixed agent if in radius
    for(f=0; f < *f_count; f++)
    {        
        float dist_x = fixed_agents[f].pos_x - moving_agents[m].pos_x;
        float dist_y = fixed_agents[f].pos_y - moving_agents[m].pos_y;

        //if(hypot(dist_x,dist_y) <= LOOKAHEAD_RADIUS) // check if in radius
            
        if((dist_x*dist_x)+(dist_y*dist_y) <= r2)
        { 
            in_radius[in_radius_count] = fixed_agents[f];
            in_radius_count++;
                
        }
    }
    // now we know all fixed agents in range, calculate new dir now
    for(i=0; i < in_radius_count; i++)
    {
        new_dir_x += in_radius[i].mov_x * INFLUENCE_FACTOR;
        new_dir_y += in_radius[i].mov_y * INFLUENCE_FACTOR;
            
        //moving_agents[m].mov_x += in_radius[i].mov_x;
        //moving_agents[m].mov_y += in_radius[i].mov_y;
    }
        
    if(in_radius_count) // only if there are some in radius
    {
        moving_agents[m].mov_x += new_dir_x;
        moving_agents[m].mov_y += new_dir_y;
        moving_agents[m].mov_x = moving_agents[m].mov_x / (in_radius_count * INFLUENCE_FACTOR + 1);
        moving_agents[m].mov_y = moving_agents[m].mov_y / (in_radius_count * INFLUENCE_FACTOR + 1);
        moving_agents[m].stat  = STAT_OCCUPIED1;
    }
    else
    {
        moving_agents[m].stat  = STAT_FREE;    
    }

    // check borders
    if ((moving_agents[m].pos_x + moving_agents[m].mov_x) >= DIM_X/2) { moving_agents[m].mov_x = -moving_agents[m].mov_x; }
    if ((moving_agents[m].pos_x + moving_agents[m].mov_x) < -DIM_X/2) { moving_agents[m].mov_x = -moving_agents[m].mov_x; }
    if ((moving_agents[m].pos_y + moving_agents[m].mov_y) >= DIM_Y/2) { moving_agents[m].mov_y = -moving_agents[m].mov_y; }
    if ((moving_agents[m].pos_y + moving_agents[m].mov_y) < -DIM_Y/2) { moving_agents[m].mov_y = -moving_agents[m].mov_y; }

    // now move agent
    moving_agents[m].pos_x += moving_agents[m].mov_x; 
    moving_agents[m].pos_y += moving_agents[m].mov_y;

    //debug
    //moving_agents[m].pos_x = gpu_run;
    
    //gpu_run++;

    
    *ret = 1;

    

}

