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

#include "oclBodySystemOpenclLaunch.h"
#include <oclUtils.h>
#include <cstdlib>
#include <cstdio>
#include <GL/glut.h>
#include "CL/cl_gl.h"

// var to hold path to executable
extern const char* cExecutablePath;

extern "C"
{
    char* clSourcefile = "oclNbodyKernel.cl";

    void AllocateNBodyArrays(cl_context cxGPUContext, cl_mem* vel, int numBodies)
    {
        // 4 floats each for alignment reasons
        unsigned int memSize = sizeof( float) * 4 * numBodies;
        
        vel[0] = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, memSize, NULL, NULL);
        vel[1] = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, memSize, NULL, NULL);
    }

    void DeleteNBodyArrays(cl_mem vel[2])
    {
        clReleaseMemObject(vel[0]);
        clReleaseMemObject(vel[1]);
    }

    void CopyArrayFromDevice(cl_command_queue cqCommandQueue, float *host, cl_mem device, cl_mem pboCL, int numBodies)
    {   
        cl_int ciErrNum;
        unsigned int size;

        if (pboCL) 
        {
            ciErrNum = clEnqueueAcquireGLObjects(cqCommandQueue, 1, &pboCL, 0, NULL, NULL);
            shrCheckError(ciErrNum, CL_SUCCESS);
        }

        size = numBodies * 4 * sizeof(float);
        ciErrNum = clEnqueueReadBuffer(cqCommandQueue, device, CL_TRUE, 0, size, host, 0, NULL, NULL);
        shrCheckError(ciErrNum, CL_SUCCESS);

        if (pboCL) 
        {
            ciErrNum = clEnqueueReleaseGLObjects(cqCommandQueue, 1, &pboCL, 0, NULL, NULL);
            shrCheckError(ciErrNum, CL_SUCCESS);
        }
    }

    void CopyArrayToDevice(cl_command_queue cqCommandQueue, cl_mem device, const float* host, int numBodies)
    {
        cl_int ciErrNum;
        unsigned int size;
        size = numBodies*4*sizeof(float);

        ciErrNum = clEnqueueWriteBuffer(cqCommandQueue, device, CL_TRUE, 0, size, host, 0, NULL, NULL);
        shrCheckError(ciErrNum, CL_SUCCESS);
    }

    cl_mem RegisterGLBufferObject(cl_context cxGPUContext, unsigned int pboGL)
    {
        return clCreateFromGLBuffer(cxGPUContext, CL_MEM_READ_WRITE, pboGL, NULL);
    }

    void UnregisterGLBufferObject(cl_mem pboCL)
    {
        clReleaseMemObject(pboCL);
    }

    void ThreadSync(cl_command_queue cqCommandQueue) 
    { 
        clFinish(cqCommandQueue); 
    }

    void IntegrateNbodySystem(cl_command_queue cqCommandQueue,
                              cl_kernel MT_kernel, cl_kernel noMT_kernel,
                              cl_mem newPos, cl_mem newVel,
                              cl_mem oldPos, cl_mem oldVel,
                              cl_mem pboCLOldPos, cl_mem pboCLNewPos,
                              float deltaTime, float damping, float softSq,
                              int numBodies, int p, int q,
                              int bUsePBO)
    {
        int sharedMemSize = p * q * sizeof(cl_float4); // 4 floats for pos
        size_t global_work_size[2];
        size_t local_work_size[2];
        cl_int ciErrNum = CL_SUCCESS;
        cl_kernel kernel;

        // When the numBodies / thread block size is < # multiprocessors 
        // (16 on G80), the GPU is underutilized. For example, with 256 threads per
        // block and 1024 bodies, there will only be 4 thread blocks, so the 
        // GPU will only be 25% utilized.  To improve this, we use multiple threads
        // per body.  We still can use blocks of 256 threads, but they are arranged
        // in q rows of p threads each.  Each thread processes 1/q of the forces 
        // that affect each body, and then 1/q of the threads (those with 
        // threadIdx.y==0) add up the partial sums from the other threads for that 
        // body.  To enable this, use the "--p=" and "--q=" command line options to
        // this example.  e.g.: "nbody.exe --n=1024 --p=64 --q=4" will use 4 
        // threads per body and 256 threads per block. There will be n/p = 16 
        // blocks, so a G80 GPU will be 100% utilized.

        if (q == 1)
        {
            kernel = MT_kernel;
        }
        else
        {
            kernel = noMT_kernel;
        }

        if (bUsePBO)
        {
            ciErrNum = clEnqueueAcquireGLObjects(cqCommandQueue, 1, &pboCLOldPos, 0, NULL, NULL);
            ciErrNum |= clEnqueueAcquireGLObjects(cqCommandQueue, 1, &pboCLNewPos, 0, NULL, NULL);
            shrCheckError(ciErrNum, CL_SUCCESS);
        }

	    ciErrNum |= clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&newPos);
        ciErrNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&newVel);
        ciErrNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&oldPos);
        ciErrNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&oldVel);   
        ciErrNum |= clSetKernelArg(kernel, 4, sizeof(cl_float), (void *)&deltaTime);
        ciErrNum |= clSetKernelArg(kernel, 5, sizeof(cl_float), (void *)&damping);
        ciErrNum |= clSetKernelArg(kernel, 6, sizeof(cl_float), (void *)&softSq);
        ciErrNum |= clSetKernelArg(kernel, 7, sizeof(cl_int), (void *)&numBodies);
        ciErrNum |= clSetKernelArg(kernel, 8, sharedMemSize, NULL);
        shrCheckError(ciErrNum, CL_SUCCESS);

        // set work-item dimensions
        local_work_size[0] = p;
        local_work_size[1] = q;
        global_work_size[0]= numBodies;
        global_work_size[1]= q;

        // execute the kernel:
        ciErrNum = clEnqueueNDRangeKernel(cqCommandQueue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
        shrCheckError(ciErrNum, CL_SUCCESS);

        if (bUsePBO)
        {
            ciErrNum = clEnqueueReleaseGLObjects(cqCommandQueue, 1, &pboCLNewPos, 0, NULL, NULL);
            ciErrNum |= clEnqueueReleaseGLObjects(cqCommandQueue, 1, &pboCLOldPos,  0, NULL, NULL);
            shrCheckError(ciErrNum, CL_SUCCESS);
        }
    }

    // Function to read in kernel from uncompiled source, create the OCL program and build the OCL program 
    // **************************************************************************************************
    int CreateProgramAndKernel(cl_context cxGPUContext, cl_device_id* cdDevices, const char *kernel_name, cl_kernel *kernel)
    {
        cl_program cpProgram;
        size_t szSourceLen;
        cl_int ciErrNum = CL_SUCCESS; 

        // Read the kernel in from file
        shrLog(LOGBOTH, 0, "\nLoading Uncompiled kernel from .cl file, using %s\n\n", clSourcefile);
        char* cPathAndFile = shrFindFilePath(clSourcefile, cExecutablePath);
        shrCheckError(cPathAndFile != NULL, shrTRUE);
        char* pcSource = oclLoadProgSource(cPathAndFile, "", &szSourceLen);
        shrCheckError(pcSource != NULL, shrTRUE);
        shrLog(LOGBOTH, 0, "oclLoadProgSource\n" ); 

        // create the program 
        cpProgram = clCreateProgramWithSource(cxGPUContext, 1, (const char **)&pcSource, &szSourceLen, &ciErrNum);
        shrCheckError(ciErrNum, CL_SUCCESS);
        shrLog(LOGBOTH, 0, "clCreateProgramWithSource\n"); 

        // Build the program with 'mad' Optimization option
#ifdef MAC
	char *flags = "-cl-mad-enable -DMAC";
#else
	char *flags = "-cl-mad-enable";
#endif
        ciErrNum = clBuildProgram(cpProgram, 0, NULL, flags, NULL, NULL);
        if (ciErrNum != CL_SUCCESS)
        {
            // write out standard error, Build Log and PTX, then cleanup and exit
            shrLog(LOGBOTH | ERRORMSG, ciErrNum, STDERROR);
            oclLogBuildInfo(cpProgram, oclGetFirstDev(cxGPUContext));
            oclLogPtx(cpProgram, oclGetFirstDev(cxGPUContext), "oclNbody.ptx");
            shrCheckError(ciErrNum, CL_SUCCESS); 
        }
        shrLog(LOGBOTH, 0, "clBuildProgram\n"); 

        // create the kernel
        *kernel = clCreateKernel(cpProgram, kernel_name, &ciErrNum);
        shrCheckError(ciErrNum, CL_SUCCESS); 
        shrLog(LOGBOTH, 0, "clCreateKernel\n"); 

	size_t wgSize;
	ciErrNum = clGetKernelWorkGroupInfo(*kernel, cdDevices[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &wgSize, NULL);
	if (wgSize == 64) {
	  shrLog(LOGBOTH, 0, 
		 "ERROR: Minimum work-group size 256 required by this application is not supported on this device.\n");
	  exit(0);
	}

        return 0;
    }
}