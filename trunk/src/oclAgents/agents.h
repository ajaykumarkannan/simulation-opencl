/*
 * Agent based Simulation
 * shared Header File
 *
 *
 * 08 - Nov - 2009  VEC     init, not functional yet
 * 08 - Mar - 2010  VEC     fully functional, and tested
 */
 
 
#ifndef _AGENTS_H_
#define _AGENTS_H_

// Thread block size
#define BLOCK_SIZE 16

// Dimensions of the Field
#define DIM_X 800
#define DIM_Y 800
#define INIT_MOVE_FACTOR_FIXED 0.1
#define INIT_MOVE_FACTOR_MOVING 0.05

// how many runs
#define RUNS_MAX 1000

// How many Agents?
#define AGENTS_FIXED_COUNT 12
#define AGENTS_MOVING_COUNT 100000       /* i think it should be a multiple of the available GPUs*/

// looking distance of moving agents
#define LOOKAHEAD_RADIUS 100
#define INFLUENCE_FACTOR 0.5

// radius of the initial circle; at least DIM_X/2
#define CIRCLE_RADIUS DIM_X/6

// print stuff
// NOTE: do not use 0 as RGB value!
#define FILENAME_LENGTH_MAX 20
#define COLOR_MAX 255
#define COLOR_MOVING_AGENTS_FREE_R 1
#define COLOR_MOVING_AGENTS_FREE_G 150
#define COLOR_MOVING_AGENTS_FREE_B 1
#define COLOR_MOVING_AGENTS_OCC1_R 1
#define COLOR_MOVING_AGENTS_OCC1_G 255
#define COLOR_MOVING_AGENTS_OCC1_B 255

#define COLOR_FIXED_AGENTS_FREE_R 200
#define COLOR_FIXED_AGENTS_FREE_G 70
#define COLOR_FIXED_AGENTS_FREE_B 70
#define COLOR_FIXED_AGENTS_OCC1_R 200
#define COLOR_FIXED_AGENTS_OCC1_G 1
#define COLOR_FIXED_AGENTS_OCC1_B 1

// how many entries per Agent
//#define AGENT_ELEMENT_COUNT 4 /* pos_x, pos_y, mov_x, mov_y - should be done by sizeof()/sizeof(float or so)*/
//#define AGENT_ELEMENT_COUNT ((sizeof(agent_vector_t)/sizeof(float))) /* like this? */

// typedefs
#define STAT_FREE 0
#define STAT_OCCUPIED1 1

typedef short t_stat;

typedef struct agent_vector {
    float  pos_x;
    float  pos_y;
    float  mov_x;
    float  mov_y;
    t_stat stat;
} agent_vector_t;

/* not sure if this a good iea to keep both in the same container */
typedef struct agent_container {
    unsigned int f_count;
    unsigned int m_count;
    agent_vector_t * f_agent_array;
    agent_vector_t * m_agent_array;
} agent_container_t; 


#endif // _AGENTS_H_

