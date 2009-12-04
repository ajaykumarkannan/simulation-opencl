/* ============================================================

Copyright (c) 2009 Advanced Micro Devices, Inc.  All rights reserved.
 
Redistribution and use of this material is permitted under the following 
conditions:
 
Redistributions must retain the above copyright notice and all terms of this 
license.
 
In no event shall anyone redistributing or accessing or using this material 
commence or participate in any arbitration or legal action relating to this 
material against Advanced Micro Devices, Inc. or any copyright holders or 
contributors. The foregoing shall survive any expiration or termination of 
this license or any agreement or access or use related to this material. 

ANY BREACH OF ANY TERM OF THIS LICENSE SHALL RESULT IN THE IMMEDIATE REVOCATION 
OF ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE THIS MATERIAL.

THIS MATERIAL IS PROVIDED BY ADVANCED MICRO DEVICES, INC. AND ANY COPYRIGHT 
HOLDERS AND CONTRIBUTORS "AS IS" IN ITS CURRENT CONDITION AND WITHOUT ANY 
REPRESENTATIONS, GUARANTEE, OR WARRANTY OF ANY KIND OR IN ANY WAY RELATED TO 
SUPPORT, INDEMNITY, ERROR FREE OR UNINTERRUPTED OPERA TION, OR THAT IT IS FREE 
FROM DEFECTS OR VIRUSES.  ALL OBLIGATIONS ARE HEREBY DISCLAIMED - WHETHER 
EXPRESS, IMPLIED, OR STATUTORY - INCLUDING, BUT NOT LIMITED TO, ANY IMPLIED 
WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
ACCURACY, COMPLETENESS, OPERABILITY, QUALITY OF SERVICE, OR NON-INFRINGEMENT. 
IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, REVENUE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED OR BASED ON ANY THEORY OF LIABILITY 
ARISING IN ANY WAY RELATED TO THIS MATERIAL, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE. THE ENTIRE AND AGGREGATE LIABILITY OF ADVANCED MICRO DEVICES, 
INC. AND ANY COPYRIGHT HOLDERS AND CONTRIBUTORS SHALL NOT EXCEED TEN DOLLARS 
(US $10.00). ANYONE REDISTRIBUTING OR ACCESSING OR USING THIS MATERIAL ACCEPTS 
THIS ALLOCATION OF RISK AND AGREES TO RELEASE ADVANCED MICRO DEVICES, INC. AND 
ANY COPYRIGHT HOLDERS AND CONTRIBUTORS FROM ANY AND ALL LIABILITIES, 
OBLIGATIONS, CLAIMS, OR DEMANDS IN EXCESS OF TEN DOLLARS (US $10.00). THE 
FOREGOING ARE ESSENTIAL TERMS OF THIS LICENSE AND, IF ANY OF THESE TERMS ARE 
CONSTRUED AS UNENFORCEABLE, FAIL IN ESSENTIAL PURPOSE, OR BECOME VOID OR 
DETRIMENTAL TO ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR 
CONTRIBUTORS FOR ANY REASON, THEN ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE 
THIS MATERIAL SHALL TERMINATE IMMEDIATELY. MOREOVER, THE FOREGOING SHALL 
SURVIVE ANY EXPIRATION OR TERMINATION OF THIS LICENSE OR ANY AGREEMENT OR 
ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE IS HEREBY PROVIDED, AND BY REDISTRIBUTING OR ACCESSING OR USING THIS 
MATERIAL SUCH NOTICE IS ACKNOWLEDGED, THAT THIS MATERIAL MAY BE SUBJECT TO 
RESTRICTIONS UNDER THE LAWS AND REGULATIONS OF THE UNITED STATES OR OTHER 
COUNTRIES, WHICH INCLUDE BUT ARE NOT LIMITED TO, U.S. EXPORT CONTROL LAWS SUCH 
AS THE EXPORT ADMINISTRATION REGULATIONS AND NATIONAL SECURITY CONTROLS AS 
DEFINED THEREUNDER, AS WELL AS STATE DEPARTMENT CONTROLS UNDER THE U.S. 
MUNITIONS LIST. THIS MATERIAL MAY NOT BE USED, RELEASED, TRANSFERRED, IMPORTED,
EXPORTED AND/OR RE-EXPORTED IN ANY MANNER PROHIBITED UNDER ANY APPLICABLE LAWS, 
INCLUDING U.S. EXPORT CONTROL LAWS REGARDING SPECIFICALLY DESIGNATED PERSONS, 
COUNTRIES AND NATIONALS OF COUNTRIES SUBJECT TO NATIONAL SECURITY CONTROLS. 
MOREOVER, THE FOREGOING SHALL SURVIVE ANY EXPIRATION OR TERMINATION OF ANY 
LICENSE OR AGREEMENT OR ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE REGARDING THE U.S. GOVERNMENT AND DOD AGENCIES: This material is 
provided with "RESTRICTED RIGHTS" and/or "LIMITED RIGHTS" as applicable to 
computer software and technical data, respectively. Use, duplication, 
distribution or disclosure by the U.S. Government and/or DOD agencies is 
subject to the full extent of restrictions in all applicable regulations, 
including those found at FAR52.227 and DFARS252.227 et seq. and any successor 
regulations thereof. Use of this material by the U.S. Government and/or DOD 
agencies is acknowledgment of the proprietary rights of any copyright holders 
and contributors, including those of Advanced Micro Devices, Inc., as well as 
the provisions of FAR52.227-14 through 23 regarding privately developed and/or 
commercial computer software.

This license forms the entire agreement regarding the subject matter hereof and 
supersedes all proposals and prior discussions and writings between the parties 
with respect thereto. This license does not affect any ownership, rights, title,
or interest in, or relating to, this material. No terms of this license can be 
modified or waived, and no breach of this license can be excused, unless done 
so in a writing signed by all affected parties. Each term of this license is 
separately enforceable. If any term of this license is determined to be or 
becomes unenforceable or illegal, such term shall be reformed to the minimum 
extent necessary in order for this license to remain in effect in accordance 
with its terms as modified by such reformation. This license shall be governed 
by and construed in accordance with the laws of the State of Texas without 
regard to rules on conflicts of law of any state or jurisdiction or the United 
Nations Convention on the International Sale of Goods. All disputes arising out 
of this license shall be subject to the jurisdiction of the federal and state 
courts in Austin, Texas, and all defenses are hereby waived concerning personal 
jurisdiction and venue of these courts.

============================================================ */


#include "QuasiRandomSequence.hpp"

/*
 * \brief Generate direction numbers
 *        v[j][32] : j dimensions each having 32 direction numbers
 */
void 
QuasiRandomSequence::generateDirectionNumbers(cl_uint nDimensions, 
                                              cl_uint* directionNumbers)
{
    cl_uint *v = directionNumbers;

    for (int dim = 0 ; dim < (int)(nDimensions); dim++)
    {
        // First dimension is a special case
        if (dim == 0)
        {
            for (int i = 0 ; i < N_DIRECTIONS ; i++)
            {
                // All m's are 1
                v[i] = 1 << (31 - i);
            }
        }
        else
        {
            int d = sobolPrimitives[dim].degree;
            // The first direction numbers (up to the degree of the polynomial)
            // are simply v[i] = m[i] / 2^i (stored in Q0.32 format)
            for (int i = 0 ; i < d ; i++)
            {
                v[i] = sobolPrimitives[dim].m[i] << (31 - i);
            }
            // The remaining direction numbers are computed as described in
            // the Bratley and Fox paper.
            // ^ = Xor operator
            // v[i] = a[1]v[i-1] ^ a[2]v[i-2] ^ ... ^ a[v-1]v[i-d+1] ^ v[i-d] ^ v[i-d]/2^d
            for (int i = d ; i < N_DIRECTIONS ; i++)
            {
                // First do the v[i-d] ^ v[i-d]/2^d part
                v[i] = v[i - d] ^ (v[i - d] >> d);
                // Now do the a[1]v[i-1] ^ a[2]v[i-2] ^ ... part
                // Note that the coefficients a[] are zero or one and for compactness in
                // the input tables they are stored as bits of a single integer. To extract
                // the relevant bit we use right shift and mask with 1.
                // For example, for a 10 degree polynomial there are ten useful bits in a,
                // so to get a[2] we need to right shift 7 times (to get the 8th bit into
                // the LSB) and then mask with 1.
                for (int j = 1 ; j < d ; j++)
                {
                    v[i] ^= (((sobolPrimitives[dim].a >> (d - 1 - j)) & 1) * v[i - j]);
                }
            }
        }
        v += N_DIRECTIONS;
    }
}


/*
 * \brief Host Initialization 
 *        Allocate and initialize memory 
 *        on the host. Print input array. 
 */

int 
QuasiRandomSequence::setupQuasiRandomSequence()
{
    /* allocate input array */
    input = (cl_uint*)malloc(nDimensions * N_DIRECTIONS * sizeof(cl_uint));

    if(input == NULL)    
    { 
        sampleCommon->error("Failed to allocate host memory. (input)");
        return SDK_FAILURE;
    }

	/* Check for dimensions */
	if(nDimensions > MAX_DIMENSIONS)
	{
		sampleCommon->error("Max allowed dimension is 12240!\n");
		return SDK_FAILURE;
	}

    /* initializa sobol direction numbers */
    generateDirectionNumbers(nDimensions, input);

    if(!quiet) 
    {
        sampleCommon->printArray<cl_uint>(
            "Input", 
            input, 
            nDimensions * N_DIRECTIONS, 
            1);
    }


    /* allocate output array */
    output = (cl_float *)malloc(nVectors * nDimensions * sizeof(cl_float));

    if(output==NULL)    
    { 
        sampleCommon->error("Failed to allocate host memory(output)");
        return SDK_FAILURE;
    }
    

    /* if verification is enabled */
    if(verify)
    {
        /* allocate memory for verification output array */
        verificationOutput = (cl_float *)malloc(nVectors * nDimensions * sizeof(cl_float));

        if(verificationOutput == NULL)   
        { 
            sampleCommon->error("Failed to allocate host memory. (verify)");
            return SDK_FAILURE;
        }

        memset(verificationOutput, 0, nVectors * nDimensions * sizeof(cl_float));
    }


    return SDK_SUCCESS;

}

int
QuasiRandomSequence::setupCL(void)
{
    cl_int status = 0;
    size_t deviceListSize;

    cl_device_type dType;
    
    if(deviceType.compare("cpu") == 0)
    {
        dType = CL_DEVICE_TYPE_CPU;
    }
    else //deviceType = "gpu" 
    {
        dType = CL_DEVICE_TYPE_GPU;
    }

    context = clCreateContextFromType(
                  0, 
                  dType, 
                  NULL, 
                  NULL, 
                  &status);
    /* 
     * if device is not set using command line arguments and opencl fails to open a 
     * context on default device GPU then it falls back to CPU 
     */
    if(status != CL_SUCCESS && dType == CL_DEVICE_TYPE_GPU && !sampleArgs->isArgSet("device"))
    {
        std::cout << "Unsupported GPU device; falling back to CPU ..." << std::endl;
        context = clCreateContextFromType(
                      0, 
                      CL_DEVICE_TYPE_CPU, 
                      NULL, 
                      NULL, 
                      &status);
    }

    if(!sampleCommon->checkVal(status, 
                  CL_SUCCESS,
                  "clCreateContextFromType failed."))
        return SDK_FAILURE;

    /* First, get the size of device list data */
    status = clGetContextInfo(
                 context, 
                 CL_CONTEXT_DEVICES, 
                 0, 
                 NULL, 
                 &deviceListSize);
    if(!sampleCommon->checkVal(
            status, 
            CL_SUCCESS,
            "clGetContextInfo failed."))
        return SDK_FAILURE;

    /* Now allocate memory for device list based on the size we got earlier */
    devices = (cl_device_id *)malloc(deviceListSize);
    if(devices==NULL) {
        sampleCommon->error("Failed to allocate memory (devices).");
        return SDK_FAILURE;
    }

    /* Now, get the device list data */
    status = clGetContextInfo(
                 context, 
                 CL_CONTEXT_DEVICES, 
                 deviceListSize, 
                 devices, 
                 NULL);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS, 
            "clGetContextInfo failed."))
        return SDK_FAILURE;

    {
        /* The block is to move the declaration of prop closer to its use */
        cl_command_queue_properties prop = 0;
        if(timing)
            prop |= CL_QUEUE_PROFILING_ENABLE;

        commandQueue = clCreateCommandQueue(
                           context, 
                           devices[0], 
                           prop, 
                           &status);
        if(!sampleCommon->checkVal(
                status,
                0,
                "clCreateCommandQueue failed."))
            return SDK_FAILURE;
    }

    /* Get Device specific Information */
    status = clGetDeviceInfo(
            devices[0],
            CL_DEVICE_MAX_WORK_GROUP_SIZE,
            sizeof(size_t),
            (void*)&maxWorkGroupSize,
            NULL);

    if(!sampleCommon->checkVal(
                status,
                CL_SUCCESS, 
                "clGetDeviceInfo CL_DEVICE_MAX_WORK_GROUP_SIZE failed."))
        return SDK_FAILURE;


    status = clGetDeviceInfo(
            devices[0],
            CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
            sizeof(cl_uint),
            (void*)&maxDimensions,
            NULL);

    if(!sampleCommon->checkVal(
                status,
                CL_SUCCESS, 
                "clGetDeviceInfo CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS failed."))
        return SDK_FAILURE;


    maxWorkItemSizes = (size_t*)malloc(maxDimensions * sizeof(size_t));
    
    status = clGetDeviceInfo(
            devices[0],
            CL_DEVICE_MAX_WORK_ITEM_SIZES,
            sizeof(size_t) * maxDimensions,
            (void*)maxWorkItemSizes,
            NULL);

    if(!sampleCommon->checkVal(
                status,
                CL_SUCCESS, 
                "clGetDeviceInfo CL_DEVICE_MAX_WORK_ITEM_SIZES failed."))
        return SDK_FAILURE;


    status = clGetDeviceInfo(
            devices[0],
            CL_DEVICE_LOCAL_MEM_SIZE,
            sizeof(cl_ulong),
            (void *)&totalLocalMemory,
            NULL);

    if(!sampleCommon->checkVal(
                status,
                CL_SUCCESS, 
                "clGetDeviceInfo CL_DEVICE_LOCAL_MEM_SIZE failed."))
        return SDK_FAILURE;

    inputBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_uint) * nDimensions * N_DIRECTIONS,
                      input, 
                      &status);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clCreateBuffer failed. (inputBuffer)"))
        return SDK_FAILURE;

    outputBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * nVectors * nDimensions,
                      output, 
                      &status);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clCreateBuffer failed. (outputBuffer)"))
        return SDK_FAILURE;

    

    /* create a CL program using the kernel source */
    streamsdk::SDKFile kernelFile;
    kernelFile.open("QuasiRandomSequence_Kernels.cl");
    const char * source = kernelFile.source().c_str();
    size_t sourceSize[] = { strlen(source) };
    program = clCreateProgramWithSource(
        context,
        1,
        &source,
        sourceSize,
        &status);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clCreateProgramWithSource failed."))
        return SDK_FAILURE;
    
    /* create a cl program executable for all the devices specified */
    status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
    if(status != CL_SUCCESS)
    {
        if(status == CL_BUILD_PROGRAM_FAILURE)
        {
            cl_int logStatus;
            char * buildLog = NULL;
            size_t buildLogSize = 0;
            logStatus = clGetProgramBuildInfo (program, 
                                            devices[0], 
                                            CL_PROGRAM_BUILD_LOG, 
                                            buildLogSize, 
                                            buildLog, 
                                            &buildLogSize);
            if(!sampleCommon->checkVal(
                        logStatus,
                        CL_SUCCESS,
                        "clGetProgramBuildInfo failed."))
                  return SDK_FAILURE;
            
            buildLog = (char*)malloc(buildLogSize);
            if(buildLog == NULL)
            {
                sampleCommon->error("Failed to allocate host memory. (buildLog)");
                return SDK_FAILURE;
            }
            memset(buildLog, 0, buildLogSize);

            logStatus = clGetProgramBuildInfo (program, 
                                            devices[0], 
                                            CL_PROGRAM_BUILD_LOG, 
                                            buildLogSize, 
                                            buildLog, 
                                            NULL);
            if(!sampleCommon->checkVal(
                        logStatus,
                        CL_SUCCESS,
                        "clGetProgramBuildInfo failed."))
            {
                  free(buildLog);
                  return SDK_FAILURE;
            }

            std::cout << " \n\t\t\tBUILD LOG\n";
            std::cout << " ************************************************\n";
            std::cout << buildLog << std::endl;
            std::cout << " ************************************************\n";
            free(buildLog);
        }

          if(!sampleCommon->checkVal(
                      status,
                      CL_SUCCESS,
                      "clBuildProgram failed."))
                return SDK_FAILURE;
    }

    /* get a kernel object handle for a kernel with the given name */
    kernel = clCreateKernel(program, "QuasiRandomSequence", &status);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clCreateKernel failed."))
        return SDK_FAILURE;

    return SDK_SUCCESS;
}



int 
QuasiRandomSequence::runCLKernels(void)
{
    cl_int   status;
    cl_event events[2];

    /* set total threads and block size */
    size_t globalThreads[1]= {nDimensions * nVectors};
    size_t localThreads[1] = {nVectors};

	if(localThreads[0] > maxWorkItemSizes[0] || localThreads[0] > maxWorkGroupSize)
	{
		std::cout<<"Unsupported: Device does not support requested number of work items.";
				 
		return SDK_FAILURE;
	}

    long long kernelsStartTime;
    long long kernelsEndTime;

    /*** Set appropriate arguments to the kernel ***/
    /* 1st argument to the kernel - outputBuffer */
    status = clSetKernelArg(
                    kernel, 
                    0, 
                    sizeof(cl_mem), 
                    (void *)&outputBuffer);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clSetKernelArg failed. (outputBuffer)"))
        return SDK_FAILURE;

    /* 2nd argument to the kernel - inputBuffer */
    status = clSetKernelArg(
                    kernel, 
                    1, 
                    sizeof(cl_mem), 
                    (void *)&inputBuffer);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clSetKernelArg failed. (inputBuffer)"))
        return SDK_FAILURE;



    /* 3rd argument to the kernel - localBuffer(shared memory) */
    status = clSetKernelArg(
                    kernel, 
                    2, 
                    32 * sizeof(cl_uint), 
                    NULL);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clSetKernelArg failed. (localBuffer)"))
        return SDK_FAILURE;

    status = clGetKernelWorkGroupInfo(kernel,
                                      devices[0],
                                      CL_KERNEL_LOCAL_MEM_SIZE,
                                      sizeof(cl_ulong),
                                      &usedLocalMemory,
                                      NULL);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clGetKernelWorkGroupInfo CL_KERNEL_LOCAL_MEM_SIZE failed."))
    {
        return SDK_FAILURE;
    }

    if(usedLocalMemory > totalLocalMemory)
    {
        std::cout << "Unsupported: Insufficient local memory on device." << std::endl;
        return SDK_FAILURE;
    }
    
    /* Enqueue a kernel run call.*/
    status = clEnqueueNDRangeKernel(
                 commandQueue,
                 kernel,
                 1,
                 NULL,
                 globalThreads,
                 localThreads,
                 0,
                 NULL,
                 &events[0]);
    
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clEnqueueNDRangeKernel failed."))
        return SDK_FAILURE;


    /* wait for the kernel call to finish execution */
    status = clWaitForEvents(1, &events[0]);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clWaitForEvents failed."))
        return SDK_FAILURE;

    clReleaseEvent(events[0]);

    if(timing)
    {
        status = clGetEventProfilingInfo(
                     events[0],
                     CL_PROFILING_COMMAND_START,
                     sizeof(long long),
                     &kernelsStartTime,
                     NULL);
        if(!sampleCommon->checkVal(
                status,
                CL_SUCCESS,
                "clGetEventProfilingInfo failed."))
            return SDK_FAILURE;
    }

    if(timing)
    {
        status = clGetEventProfilingInfo(
                     events[0],
                     CL_PROFILING_COMMAND_END,
                     sizeof(long long),
                     &kernelsEndTime,
                     NULL);
        if(!sampleCommon->checkVal(
                status,
                CL_SUCCESS,
                "clGetEventProfilingInfo failed."))
            return SDK_FAILURE;

        /* Compute total time (also convert from nanoseconds to seconds) */
        totalTime = (double)(kernelsEndTime - kernelsStartTime)/1e9;
    }


    /* Enqueue readBuffer*/
    status = clEnqueueReadBuffer(
                commandQueue,
                outputBuffer,
                CL_TRUE,
                0,
                nDimensions * nVectors * sizeof(cl_float),
                output,
                0,
                NULL,
                &events[1]);
    if(!sampleCommon->checkVal(
    		status,
    		CL_SUCCESS,
    		"clEnqueueReadBuffer failed."))
    	return SDK_FAILURE;

  
    
    clFinish(commandQueue);

    clReleaseEvent(events[1]);
   
    return SDK_SUCCESS;
}


int 
QuasiRandomSequence::initialize()
{
    // Call base class Initialize to get default configuration
    if(!this->SDKSample::initialize())
        return SDK_FAILURE;

	streamsdk::Option* array_length = new streamsdk::Option;
    if(!array_length)
    {
        sampleCommon->error("Memory allocation error.\n");
        return SDK_FAILURE;
    }

	array_length->_sVersion = "d";
    array_length->_lVersion = "dimension";
    array_length->_description = "Number of dimensions";
    array_length->_type = streamsdk::CA_ARG_INT;
    array_length->_value = &nDimensions;
    sampleArgs->AddOption(array_length);

	array_length->_sVersion = "x";
    array_length->_lVersion = "vector";
    array_length->_description = "Number of vectors";
    array_length->_type = streamsdk::CA_ARG_INT;
    array_length->_value = &nVectors;
    sampleArgs->AddOption(array_length);

    return SDK_SUCCESS;
}

int 
QuasiRandomSequence::setup()
{  
    if(setupQuasiRandomSequence()!=SDK_SUCCESS)
        return SDK_FAILURE;

    /* create and initialize timers */
    int timer = sampleCommon->createTimer();
    sampleCommon->resetTimer(timer);
    sampleCommon->startTimer(timer);

    if(setupCL()!=SDK_SUCCESS)
		return SDK_FAILURE;

    sampleCommon->stopTimer(timer);
    /* Compute setup time */
    setupTime = (double)(sampleCommon->readTimer(timer));

    return SDK_SUCCESS;
}


int QuasiRandomSequence::run()
{
    /* create and initialize timers */
    int timer = sampleCommon->createTimer();
    sampleCommon->resetTimer(timer);
    sampleCommon->startTimer(timer);    

    /* Arguments are set and execution call is enqueued on command buffer */
    if(runCLKernels()!=SDK_SUCCESS)
		return SDK_FAILURE;

    sampleCommon->stopTimer(timer);    
    /* Compute kernel time */
    kernelTime = (double)(sampleCommon->readTimer(timer));

    if(!quiet) 
    {
        sampleCommon->printArray<cl_float>("Output", 
                                           output, 
                                           nDimensions * nVectors, 1);
    }

    return SDK_SUCCESS;
}


void 
QuasiRandomSequence::quasiRandomSequenceCPUReference()
{

    for(int j=0; j < (int)nDimensions; j++)
    {
        for(int i=0; i < (int)nVectors; i++)
        {
            unsigned int temp = 0;

            for(int k=0; k < 32; k++)
            {
                int mask = (int)(pow(2, (double)k));

                temp ^= ((i & mask) >> k) * input[j * 32 + k];
            }

            if(i==0 && j==0)
            {
                verificationOutput[j * nVectors + i] = 0;
            }
            else
            {
                verificationOutput[j * nVectors + i] = (cl_float)(temp / pow(2, (double)32));
            }

        }
    }
    
}





int 
QuasiRandomSequence::verifyResults()
{

    if(verify)
    {
        /* reference implementation */

        quasiRandomSequenceCPUReference();

        /* compare the results and see if they match */
        if(sampleCommon->compare(output, verificationOutput, nDimensions * nVectors))
        {
            std::cout<<"Passed!\n";
            return SDK_SUCCESS;
        }
        else
        {
            std::cout<<"Failed\n";
            return SDK_FAILURE;
        }
    }

    return SDK_SUCCESS;
}

void 
QuasiRandomSequence::printStats()
{
    std::string strArray[2] = {"Elements", "Time(sec)"};
    std::string stats[2];

    int length = nDimensions * nVectors;
    
    totalTime = setupTime + kernelTime;
    stats[0]  = sampleCommon->toString(length, std::dec);
    stats[1]  = sampleCommon->toString(totalTime, std::dec);
    
    this->SDKSample::printStats(strArray, stats, 2);

}

int 
QuasiRandomSequence::cleanup()
{
    /* Releases OpenCL resources (Context, Memory etc.) */
    cl_int status;

    status = clReleaseKernel(kernel);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseKernel failed."))
        return SDK_FAILURE;

    status = clReleaseProgram(program);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseProgram failed."))
        return SDK_FAILURE;

    status = clReleaseMemObject(inputBuffer);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseMemObject failed."))
        return SDK_FAILURE;

    status = clReleaseMemObject(outputBuffer);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseMemObject failed."))
        return SDK_FAILURE;

    status = clReleaseCommandQueue(commandQueue);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseCommandQueue failed."))
        return SDK_FAILURE;

    status = clReleaseContext(context);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clReleaseContext failed."))
        return SDK_FAILURE;

    /* release program resources (input memory etc.) */
    if(input) 
        free(input);

    if(output)
        free(output);

    if(verificationOutput) 
        free(verificationOutput);

    if(devices)
        free(devices);

    if(maxWorkItemSizes)
        free(maxWorkItemSizes);

    return SDK_SUCCESS;
}

int 
main(int argc, char * argv[])
{
    QuasiRandomSequence clQuasiRandomSequence("OpenCL Quasi Random Sequence");

    if(clQuasiRandomSequence.initialize()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(!clQuasiRandomSequence.parseCommandLine(argc, argv))
        return SDK_FAILURE;
    if(clQuasiRandomSequence.setup()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clQuasiRandomSequence.run()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clQuasiRandomSequence.verifyResults()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clQuasiRandomSequence.cleanup()!=SDK_SUCCESS)
        return SDK_FAILURE;
	clQuasiRandomSequence.printStats();

    return SDK_SUCCESS;
}

