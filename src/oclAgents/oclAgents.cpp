/*
 * Agent based Simulation
 * Host File
 *
 *
 * 08 - Nov - 2009  VEC     init, not functional yet
 */

// standard utilities and system includes
#include <oclUtils.h>
#include <math.h>
#include <stdlib.h>

// project include
#include "agents.h"

// max GPU's to manage for multi-GPU parallel compute
const unsigned int MAX_GPU_COUNT = 8;

// global variables
cl_context cxGPUContext;
cl_kernel multiplicationKernel[MAX_GPU_COUNT];
cl_command_queue commandQueue[MAX_GPU_COUNT];

////////////////////////////////////////////////////////////////////////////////
// declaration, forward
int runTest(int argc, const char** argv);
void printDiff(float*, float*, int, int);
void matrixMulGPU(cl_uint ciDeviceCount, cl_mem h_A, float* h_B_data, unsigned int mem_size_B, float* h_C );

extern "C"
void computeGold(float*, const float*, const float*, unsigned int, unsigned int, unsigned int);

////////////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////////////

double executionTime(cl_event &event)
{
    cl_ulong start, end;
    
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    
    return (double)1.0e-9 * (end - start); // convert nanoseconds to seconds on return
}


void f_agents_init_circle(agent_container_t * container)
{
   // angle differences in rad
   float phi = (2*M_PI)/(container->f_count);  

    
   for(unsigned int i = 0; i < container->f_count; i++)
   {
        float * pos_x = &(container->f_agent_array[i].pos_x);
        float * pos_y = &(container->f_agent_array[i].pos_y);

        float * mov_x = &(container->f_agent_array[i].mov_x);
        float * mov_y = &(container->f_agent_array[i].mov_y);
    
        // calc positions
        *pos_x = (-1)*CIRCLE_RADIUS*sin(phi*i);
        *pos_y = CIRCLE_RADIUS*cos(phi*i);
        
        // calc moving directions
        *mov_x = (-1)*(*pos_y);
        *mov_y = (*pos_x);
        
        // output        
        shrLog(LOGBOTH, 0, "Element %d: pos x%f y%f; direction x%f y%f\n", i, *pos_x, *pos_y, *mov_x, *mov_y);
   }
    
}

void m_agents_init_rand(agent_container_t * container)
{
    
    srand( time(NULL) );
    
    for(unsigned int i = 0; i < container->m_count; i++)
    {
        container->m_agent_array[i].pos_x = (rand() % DIM_X) - DIM_X/2; 
        container->m_agent_array[i].pos_y = (rand() % DIM_Y) - DIM_Y/2;
        container->m_agent_array[i].mov_x = (rand() % DIM_X) - DIM_X/2;
        container->m_agent_array[i].mov_y = (rand() % DIM_Y) - DIM_Y/2;
    }
}


void m_agents_init_null(agent_container_t * container)
{
    
    for(unsigned int i = 0; i < container->m_count; i++)
    {
        container->m_agent_array[i].pos_x = 00.00; 
        container->m_agent_array[i].pos_y = 00.00;
        container->m_agent_array[i].mov_x = 00.00;
        container->m_agent_array[i].mov_y = 00.00;
    }
}

	
void print_moving_agents(agent_vector_t * m_agents, unsigned int count, const char * prefix)
{
    unsigned int i;
    
    if(prefix)
    { shrLog(LOGBOTH, 0, "%s\n",prefix); } 
    
    for(i=0; i < count; i++)
    {
        agent_vector_t * a = &m_agents[i];
        shrLog(LOGBOTH, 0, "Agent %d pos x%f y%f; direction x%f y%f\n", i, a->pos_x, a->pos_y, a->mov_x, a->mov_y);    
    }    
    
}

void print_ppm(agent_container_t * container, unsigned int run_nr)  // do on gpu later on!
{
    char ppm[DIM_X][DIM_Y][3];
    char ppm_out[(DIM_X*DIM_Y*3)+1];
    
    char filename[FILENAME_LENGTH_MAX];
    
    memset(&ppm[0][0], 1, DIM_X*DIM_Y*3);
    
    // fill mobile agents
    for(unsigned int i = 0; i < container->m_count; i++)
    {
        int x = container->m_agent_array[i].pos_x;
        int y = container->m_agent_array[i].pos_y;
        
        if ((x+50) >= 0 && (x+50) < DIM_X && (y+50) >= 0 && (y+50) < DIM_Y) // check for safety
        {    ppm[x][y][0] = COLOR_MOBILE_AGENTS_R;
             ppm[x][y][1] = COLOR_MOBILE_AGENTS_G;
             ppm[x][y][2] = COLOR_MOBILE_AGENTS_B; }
        else
        { shrLog(LOGBOTH,0, "ERROR: ppm_gen: agent out of range with x%d y%d\n", x,y); }
        
    }
    
    memcpy(ppm_out, ppm, DIM_X * DIM_Y * 3);
    ppm_out[DIM_X*DIM_Y*3] = NULL;
        
    snprintf(filename, FILENAME_LENGTH_MAX, "out_%d.ppm", run_nr);
    
    FILE* f; // create a new file pointer
    if((f=fopen(filename,"w"))==NULL) 
    { shrLog(LOGBOTH,0, "ERROR: could not open file\n"); }    
    
    //ppm header
    fprintf(f,"P6\n%d %d %d\n", DIM_X, DIM_Y, COLOR_MAX);
    
    // ppm binary info
    fputs(ppm_out,f);
    
    fclose(f);
        
}


////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int main(int argc, const char** argv)
{
    // start the logs
    shrSetLogFileName ("oclMatrixMul.txt");
    shrLog(LOGBOTH, 0, "%s Starting...\n\n", argv[0]); 

    // run the code
    if (runTest(argc, argv) != 0)
    {
        shrLog(LOGBOTH, 0, "TEST FAILED !!!\n\n");
    }

    // finish
    shrEXIT(argc, argv);
}

//void AgentsGPU(cl_uint ciDeviceCount, cl_mem buffer_fixed, cl_mem buffer_moving, unsigned int f_count, unsigned int m_count)
void AgentsGPU(cl_uint ciDeviceCount, agent_container_t * container)
{

    unsigned int f_count = container->f_count;
    unsigned int m_count = container->m_count;
    
    int ret_code[MAX_GPU_COUNT];
    
    agent_vector_t * f_host = container->f_agent_array;
    agent_vector_t * m_host = container->m_agent_array;

    agent_vector_t result_fixed[f_count];
    agent_vector_t result_moving[m_count];

    cl_mem GPUmem_fixed[MAX_GPU_COUNT];
    cl_mem GPUmem_moving[MAX_GPU_COUNT];
    cl_mem GPUmem_fcount[MAX_GPU_COUNT];
    cl_mem GPUmem_mcount[MAX_GPU_COUNT];
    cl_mem GPUmem_ret[MAX_GPU_COUNT];

    cl_event GPUDone[MAX_GPU_COUNT];
    
    cl_int ret = 0;

    // Start the computation on each available GPU
    
    // Create buffers for each GPU
    // Each GPU will compute sizePerGPU rows of the result
    int sizePerGPU = m_count / ciDeviceCount;

    shrLog(LOGBOTH, 0, "every GPU does %d moving agents\n", sizePerGPU);    

    int workOffset[MAX_GPU_COUNT];
    int workSize[MAX_GPU_COUNT];

    workOffset[0] = 0;
    for(unsigned int i=0; i < ciDeviceCount; ++i) 
    {
        // Input buffer
        workSize[i] = (i != (ciDeviceCount - 1)) ? sizePerGPU : (m_count - workOffset[i]);        /* last unit might not have to do full size */
        shrLog(LOGBOTH, 0, "GPU %d does %d moving agents\n", i, workSize[i]);  

        //init errcode
        ret_code[i] = 0;

        GPUmem_fixed[i] = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, f_count * sizeof(agent_vector_t), NULL,NULL);
        GPUmem_moving[i] = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, workSize[i] * sizeof(agent_vector_t), NULL,NULL);
        GPUmem_fcount[i] = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(unsigned int), &f_count,NULL);
        GPUmem_mcount[i] = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(unsigned int), &workSize[i],NULL);
        GPUmem_ret[i] = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(int), &ret_code[i],NULL);

        // Copy fixed agents and only assigned moving agents from host to device
        clEnqueueWriteBuffer(commandQueue[i], GPUmem_fixed[i], CL_TRUE, 0, f_count * sizeof(agent_vector_t), f_host, NULL, NULL, NULL);
        clEnqueueWriteBuffer(commandQueue[i], GPUmem_moving[i], CL_TRUE, workOffset[i], m_count * sizeof(agent_vector_t), m_host, NULL, NULL, NULL);

        
        /* clEnqueueCopyBuffer(commandQueue[i], buffer_fixed, GPUmem_fixed[i], 0, 0, f_count * sizeof(agent_vector_t), 0, NULL, NULL);       
        clEnqueueCopyBuffer(commandQueue[i], buffer_moving, GPUmem_moving[i], workOffset[i] * sizeof(agent_vector_t), 0, workSize[i] * sizeof(agent_vector_t), 0, NULL, NULL);       */
              
        // set the args values
        clSetKernelArg(multiplicationKernel[i], 0, sizeof(cl_mem), (void *)&(GPUmem_fixed[i]));  /* address of fixed agents */
        clSetKernelArg(multiplicationKernel[i], 1, sizeof(cl_mem), (void *)&(GPUmem_moving[i])); /* address of moving agents */
        clSetKernelArg(multiplicationKernel[i], 2, sizeof(cl_mem), (void *)&GPUmem_fcount);       /* count of fixed agents */
        clSetKernelArg(multiplicationKernel[i], 3, sizeof(cl_mem), (void *)&GPUmem_mcount);   /* count of moving agents */
        clSetKernelArg(multiplicationKernel[i], 4, sizeof(cl_mem), (void *)&GPUmem_ret[i]);   /* count of moving agents */

        if(i+1 < ciDeviceCount)
            workOffset[i + 1] = workOffset[i] + workSize[i];
    }
    
    // Start timer and launch kernels on devices
    /* shrDeltaT(0); */
    
    for(unsigned int run = 0; run < RUNS_MAX; run++)
    {
    
        for(unsigned int i = 0; i < ciDeviceCount; i++) 
        {
            ret = clEnqueueTask(commandQueue[i], multiplicationKernel[i], 0, NULL, &GPUDone[i]);
            if (ret != CL_SUCCESS)
            { shrLog(LOGBOTH, 0, "enqueue tsk failed with ret %d\n",ret); }
            (void)clFlush(commandQueue[i]);
        }
    
        // CPU sync with GPU
        clWaitForEvents(ciDeviceCount, GPUDone);

        for(unsigned int i = 0; i < ciDeviceCount; i++) 
        {    
            clEnqueueReadBuffer(commandQueue[i], GPUmem_fixed[i], CL_TRUE, 0, 
                            f_count * sizeof(agent_vector_t), result_fixed, 0, NULL, NULL);
            // blocking copy of result from device to host
            clEnqueueReadBuffer(commandQueue[i], GPUmem_moving[i], CL_TRUE, workOffset[i] * sizeof(agent_vector_t), 
                            workSize[i] * sizeof(agent_vector_t), &result_moving[workOffset[i]], 0, NULL, &GPUDone[i]);
            clEnqueueReadBuffer(commandQueue[i], GPUmem_moving[i], CL_TRUE, workOffset[i] * sizeof(agent_vector_t), 
                            workSize[i] * sizeof(agent_vector_t), &(container->m_agent_array[workOffset[i]]), 0, NULL, &GPUDone[i]);
    
            shrLog(LOGBOTH, 0, "ret from Device %d is %d\n", i, ret_code[i]);
    
            // copy back to mem that will be displayed
            //memcpy(&container->m_agent_array[workOffset[i]], &result_moving[workOffset[i]], workSize[i] * sizeof(agent_vector_t));
        }

        // CPU sync with GPU
        clWaitForEvents(ciDeviceCount, GPUDone);

        print_ppm(container, run);

        //print_moving_agents(result_fixed, f_count, "FIXED AGENTS");
        print_moving_agents(result_moving, m_count, "MOVING AGENTS");

    }

    // stop and log timer 
    #ifdef GPU_PROFILING
        double dSeconds = shrDeltaT(0);
        double dSize = ((double)WA * (double)HA * (double)WB * (double)HB);
        shrLog(LOGBOTH | MASTER, 0, "oclMatrixMul, Throughput = %.4f, Time = %.5f, Size = %.0f, NumDevsUsed = %d, Workgroup = %u\n", 
                1.0e-9 * dSize/dSeconds, dSeconds, dSize, ciDeviceCount, localWorkSize[0] * localWorkSize[1]);

        // Print kernel timing per GPU
        for(unsigned int i = 0; i < ciDeviceCount; i++) 
        {    
            shrLog(LOGBOTH, 0, "  Kernel execution time on GPU %d \t: %.5f s\n", i, executionTime(GPUExecution[i]));
        }
        shrLog(LOGBOTH, 0, "\n");
     #endif

    // Release mem and event objects    
    for(unsigned int i = 0; i < ciDeviceCount; i++) 
    {
        clReleaseMemObject(GPUmem_fixed[i]);
        clReleaseMemObject(GPUmem_moving[i]);
	    clReleaseMemObject(GPUmem_fcount[i]);
	    clReleaseMemObject(GPUmem_mcount[i]);
	    clReleaseEvent(GPUDone[i]);
    }
}


////////////////////////////////////////////////////////////////////////////////
//! Run a simple test for 
////////////////////////////////////////////////////////////////////////////////
int runTest(int argc, const char** argv)
{
    cl_uint ciDeviceCount = 0;
    cl_int ciErrNum = CL_SUCCESS;

    // create the OpenCL context on available GPU devices
    cxGPUContext = clCreateContextFromType(0, CL_DEVICE_TYPE_GPU, NULL, NULL, &ciErrNum);
    if (ciErrNum != CL_SUCCESS)
    {
        shrLog(LOGBOTH, 0, "Error: Failed to create OpenCL context! return with: %d\n", ciErrNum);
        return ciErrNum;
    }

    if(shrCheckCmdLineFlag(argc, (const char**)argv, "device"))
    {
        // User specified GPUs
        char* deviceList;
        char* deviceStr;
        shrGetCmdLineArgumentstr(argc, (const char**)argv, "device", &deviceList);

        #ifdef WIN32
            deviceStr = strtok_s (deviceList," ,.-", &next_token);
        #else
            deviceStr = strtok (deviceList," ,.-");
        #endif   
        while(deviceStr != NULL) 
        {
            // get and print the device for this queue
            cl_device_id device = oclGetDev(cxGPUContext, atoi(deviceStr));
            shrLog(LOGBOTH, 0, "Device %d:\n", ciDeviceCount);
            oclPrintDevName(LOGBOTH, device);            
           
            // create command queue
            commandQueue[ciDeviceCount] = clCreateCommandQueue(cxGPUContext, device, 0, &ciErrNum);
            if (ciErrNum != CL_SUCCESS)
            {
                shrLog(LOGBOTH, 0, " Error %i in clCreateCommandQueue call !!!\n\n", ciErrNum);
                return ciErrNum;
            }

            #ifdef GPU_PROFILING
                ciErrNum = clSetCommandQueueProperty(commandQueue[ciDeviceCount], CL_QUEUE_PROFILING_ENABLE, CL_TRUE, NULL);
                if (ciErrNum != CL_SUCCESS)
                {
                    shrLog(LOGBOTH, 0, " Error %i in clSetCommandQueueProperty call !!!\n\n", ciErrNum);
                    return ciErrNum;
                }
            #endif
                
             ++ciDeviceCount;

            #ifdef WIN32
                deviceStr = strtok_s (NULL," ,.-", &next_token);
            #else            
                deviceStr = strtok (NULL," ,.-");
            #endif
        }

        free(deviceList);
    } 
    else 
    {
        // Find out how many GPU's to compute on all available GPUs
	    size_t nDeviceBytes;
	    ciErrNum |= clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, 0, NULL, &nDeviceBytes);
	    ciDeviceCount = (cl_uint)nDeviceBytes/sizeof(cl_device_id);

        if (ciErrNum != CL_SUCCESS)
        {
            shrLog(LOGBOTH, 0, " Error %i in clGetDeviceIDs call !!!\n\n", ciErrNum);
            return ciErrNum;
        }
        else if (ciDeviceCount == 0)
        {
            shrLog(LOGBOTH, 0, " There are no devices supporting OpenCL (return code %i)\n\n", ciErrNum);
            return -1;
        } 

        // create command-queues
        for(unsigned int i = 0; i < ciDeviceCount; ++i) 
        {
            // get and print the device for this queue
            cl_device_id device = oclGetDev(cxGPUContext, i);
            shrLog(LOGBOTH, 0, "Device %d:\n", i);
            oclPrintDevName(LOGBOTH, device);            
            
            // create command queue
            commandQueue[i] = clCreateCommandQueue(cxGPUContext, device, 0, &ciErrNum);
            if (ciErrNum != CL_SUCCESS)
            {
                shrLog(LOGBOTH, 0, " Error %i in clCreateCommandQueue call !!!\n\n", ciErrNum);
                return ciErrNum;
            }
            #ifdef GPU_PROFILING
                clSetCommandQueueProperty(commandQueue[i], CL_QUEUE_PROFILING_ENABLE, CL_TRUE, NULL);
            #endif
        }
    }
    
    // allocate the memory for the fixed agents
    // the array keeping all the fixed agents
    agent_vector_t * f_agent_array = (agent_vector_t *)malloc(AGENTS_FIXED_COUNT*sizeof(agent_vector_t));
    shrLog(LOGBOTH, 0, " alloced %d bytes host memory for %d fixed agents\n", AGENTS_FIXED_COUNT*sizeof(agent_vector_t), AGENTS_FIXED_COUNT);

    // array keping all the moving agents
    agent_vector_t * m_agent_array = (agent_vector_t *)malloc(AGENTS_MOVING_COUNT*sizeof(agent_vector_t));
    shrLog(LOGBOTH, 0, " alloced %d bytes host memory for %d moving agents\n", AGENTS_MOVING_COUNT*sizeof(agent_vector_t), AGENTS_MOVING_COUNT);

    // container init
    agent_container_t * agent_container = (agent_container_t *)malloc(sizeof(agent_container_t));
    shrLog(LOGBOTH, 0, " alloced %d bytes host memory for the container\n", sizeof(agent_container_t));
    
    agent_container->f_count = AGENTS_FIXED_COUNT;
    agent_container->m_count = AGENTS_MOVING_COUNT;
    agent_container->f_agent_array = f_agent_array;
    agent_container->m_agent_array = m_agent_array;
    
    // create OpenCL buffers pointing to the host memory
    /* cl_mem buffer_fixed = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
				    AGENTS_FIXED_COUNT*sizeof(agent_vector_t), f_agent_array, &ciErrNum);
    if (ciErrNum != CL_SUCCESS)
    {
        shrLog(LOGBOTH, 0, "Error: clCreateBuffer for fixed agents\n");
        return ciErrNum;
    }
    cl_mem buffer_moving = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
				    AGENTS_MOVING_COUNT*sizeof(agent_vector_t), m_agent_array, &ciErrNum);
    if (ciErrNum != CL_SUCCESS)
    {
        shrLog(LOGBOTH, 0, "Error: clCreateBuffer for moving agents\n");
        return ciErrNum;
    } */
    
    
    // init in circe
    f_agents_init_circle(agent_container);
    //m_agents_init_null(agent_container);
    m_agents_init_rand(agent_container);
    

    // Program Setup
    size_t program_length;
    const char* header_path = shrFindFilePath("agents.h", argv[0]);
    char* header = oclLoadProgSource(header_path, "", &program_length);
    if(!header)
    {
        shrLog(LOGBOTH, 0, "Error: Failed to load the header %s!\n", header_path);
        return -1000;
    }
    const char* source_path = shrFindFilePath("agents.cl", argv[0]);
    char *source = oclLoadProgSource(source_path, header, &program_length);
    if(!source)
    {
        shrLog(LOGBOTH, 0, "Error: Failed to load compute program %s!\n", source_path);
        return -2000;
    }

    // create the program
    cl_program cpProgram = clCreateProgramWithSource(cxGPUContext, 1, (const char **)&source, 
                                                    &program_length, &ciErrNum);
    if (ciErrNum != CL_SUCCESS)
    {
        shrLog(LOGBOTH, 0, "Error: Failed to create program\n");
        return ciErrNum;
    }
    free(header);
    free(source);
    
    // build the program
    ciErrNum = clBuildProgram(cpProgram, 0, NULL, "-cl-mad-enable", NULL, NULL);
    if (ciErrNum != CL_SUCCESS)
    {
        // write out standard error, Build Log and PTX, then return error
        shrLog(LOGBOTH | ERRORMSG, ciErrNum, STDERROR);
        oclLogBuildInfo(cpProgram, oclGetFirstDev(cxGPUContext));
        oclLogPtx(cpProgram, oclGetFirstDev(cxGPUContext), "oclAgents.ptx");
        return ciErrNum;
    }

    // write out PTX if requested on the command line
    if(shrCheckCmdLineFlag(argc, argv, "dump-ptx") )
    {
        oclLogPtx(cpProgram, oclGetFirstDev(cxGPUContext), "oclAgents.ptx");
    }

    // Create Kernel
    for(unsigned int i=0; i<ciDeviceCount; ++i) {
        shrLog(LOGBOTH, 0, "Creating Kernel for Device %d of total %d devices.\n", i, ciDeviceCount);
        multiplicationKernel[i] = clCreateKernel(cpProgram, "agents", &ciErrNum);
        if (ciErrNum != CL_SUCCESS)
        {
            shrLog(LOGBOTH, 0, "Error: Failed to create kernel err %d\n", ciErrNum);
            return ciErrNum;
        }
    }
        
    // Run multiplication on 1..deviceCount GPUs to compare improvement
    shrLog(LOGBOTH, 0, "\nRunning Computations on 1 - %d GPU's...\n", ciDeviceCount);
    for(unsigned int k = 1; k <= ciDeviceCount; ++k) 
    {
        AgentsGPU(k, agent_container);
        /* AgentsGPU(k, buffer_fixed, buffer_moving, agent_container->f_count, agent_container->m_count); */
    }

    // clean up OCL resources
    // clReleaseMemObject(buffer_moving);
    // clReleaseMemObject(buffer_fixed);
    for(unsigned int k = 0; k < ciDeviceCount; ++k) 
    {
        clReleaseKernel( multiplicationKernel[k] );
        clReleaseCommandQueue( commandQueue[k] );
    }
    clReleaseProgram(cpProgram);
    ciErrNum = clReleaseContext(cxGPUContext);
    if( ciErrNum != CL_SUCCESS) 
        shrLog(LOGBOTH, 0, "Error: Failed to release context: %d\n", ciErrNum);

    // clean up memory
    free(f_agent_array);
    free(m_agent_array);
    free(agent_container);
    
    return 0;
}

void printDiff(float *data1, float *data2, int width, int height)
{
  int i,j,k;
  int error_count=0;
  for (j=0; j<height; j++) {
    for (i=0; i<width; i++) {
      k = j*width+i;
      if ( fabs(data1[k] - data2[k]) < 1e-5) {
          shrLog(LOGBOTH, 0, "diff(%d,%d) CPU=%.4f, GPU=%.4f \n", i,j, data1[k], data2[k]);
          error_count++;
      }

    }
      shrLog(LOGBOTH, 0, "\n");
  }
  shrLog(LOGBOTH, 0, " \nTotal Errors = %d \n", error_count);
}
