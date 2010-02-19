/*
 * Copyright 1993-2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and 
 * proprietary rights in and to this software and related documentation. 
 * Any use, reproduction, disclosure, or distribution of this software 
 * and related documentation without an express license agreement from
 * NVIDIA Corporation is strictly prohibited.
 *
 * Please refer to the applicable NVIDIA end user license agreement (EULA) 
 * associated with this source code for terms and conditions that govern 
 * your use of this NVIDIA software.
 * 
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
#define RUNS_MAX 400

// How many Agents?
#define AGENTS_FIXED_COUNT 12       /* i think it should be a multiple of the available GPUs*/
#define AGENTS_MOVING_COUNT 800

// looking distance of moving agents
#define LOOKAHEAD_RADIUS 150
#define INFLUENCE_FACTOR 1

// radius of the initial circle
#define CIRCLE_RADIUS 100

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
#define AGENT_ELEMENT_COUNT ((sizeof(agent_vector_t)/sizeof(float))) /* like this? */

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

