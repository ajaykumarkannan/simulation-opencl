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


#include "DCT.hpp"

using namespace dct;

int 
DCT::setupDCT()
{
    cl_uint inputSizeBytes;

    /* allocate and init memory used by host */
    inputSizeBytes = width * height * sizeof(cl_float);
    input = (cl_float *) malloc(inputSizeBytes);
    if(input==NULL)    
    { 
        sampleCommon->error("Failed to allocate host memory. (input)");
        return SDK_FAILURE;
    }

    cl_uint outputSizeBytes =  width * height * sizeof(cl_float);
    output = (cl_float *)malloc(outputSizeBytes);
    if(output==NULL)    
    { 
        sampleCommon->error("Failed to allocate host memory. (output)");
        return SDK_FAILURE;
    }

    /* random initialisation of input */
    sampleCommon->fillRandom<cl_float>(input, width, height, 0, 255);

    /* 
     * Unless quiet mode has been enabled, print the INPUT array.
     */
    if(!quiet) 
    {
        sampleCommon->printArray<cl_float>(
            "Input", 
            input, 
            width, 
            1);        
    }

    return SDK_SUCCESS;
}

int
DCT::setupCL(void)
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
            "clGetGetContextInfo failed."))
        return SDK_FAILURE;

    /* Get Device specific Information */
    status = clGetDeviceInfo(
            devices[0],
            CL_DEVICE_MAX_WORK_GROUP_SIZE,
            sizeof(size_t),
            (void *)&maxWorkGroupSize,
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
            (void *)&maxDimensions,
            NULL);

    if(!sampleCommon->checkVal(
                status,
                CL_SUCCESS, 
                "clGetDeviceInfo CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS failed."))
        return SDK_FAILURE;


    maxWorkItemSizes = (size_t *)malloc(maxDimensions*sizeof(size_t));
    
    status = clGetDeviceInfo(
            devices[0],
            CL_DEVICE_MAX_WORK_ITEM_SIZES,
            sizeof(size_t)*maxDimensions,
            (void *)maxWorkItemSizes,
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
                "clGetDeviceInfo CL_DEVICE_LOCAL_MEM_SIZES failed."))
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

    inputBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * width * height,
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
                      sizeof(cl_float) * width * height,
                      output, 
                      &status);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clCreateBuffer failed. (outputBuffer)"))
        return SDK_FAILURE;

    dctBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * blockSize,
                      dct8x8, 
                      &status);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clCreateBuffer failed. (dctBuffer)"))
        return SDK_FAILURE;
   
    /* create a CL program using the kernel source */
    streamsdk::SDKFile kernelFile;
    kernelFile.open("DCT_Kernels.cl");
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
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clBuildProgram failed."))
        return SDK_FAILURE;

    /* get a kernel object handle for a kernel with the given name */
    kernel = clCreateKernel(program, "DCT", &status);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clCreateKernel failed."))
        return SDK_FAILURE;

    return SDK_SUCCESS;
}


int 
DCT::runCLKernels(void)
{
    cl_int   status;
    cl_event events[2];

    size_t globalThreads[2] = {width, height};
    size_t localThreads[2] = {blockWidth, blockWidth};

    long long kernelsStartTime;
    long long kernelsEndTime;

    status =  clGetKernelWorkGroupInfo(
                    kernel,
                    devices[0],
                    CL_KERNEL_LOCAL_MEM_SIZE,
                    sizeof(cl_ulong),
                    &usedLocalMemory,
                    NULL);
    if(!sampleCommon->checkVal(
                status,
                CL_SUCCESS,
                "clGetKernelWorkGroupInfo failed.(usedLocalMemory)"))
        return SDK_FAILURE;

    availableLocalMemory = totalLocalMemory - usedLocalMemory;

    neededLocalMemory    = blockWidth * blockWidth* sizeof(cl_float); 

    if(neededLocalMemory > availableLocalMemory)
    {
        std::cout << "Unsupported: Insufficient local memory on device." << std::endl;
        return SDK_SUCCESS;
    }

    if(localThreads[0]                 > maxWorkItemSizes[0] ||
       localThreads[1]                 > maxWorkItemSizes[1] ||
       localThreads[0]*localThreads[1] > maxWorkGroupSize    )
    {
        std::cout << "Unsupported: Device does not support requested number of work items."<<std::endl;
        return SDK_SUCCESS;
    }

	/*** Set appropriate arguments to the kernel ***/
    /** 
     * 1st argument to the kernel which stores the output of the DCT 
     */
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

    /**
     * 2nd argument to the kernel , the input matrix 
     */
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

    /**
     * 3rd argument to the kernel , the dct8x8 cosine basis function matrix 
     */
    status = clSetKernelArg(
                    kernel, 
                    2, 
                    sizeof(cl_mem), 
                    (void *)&dctBuffer);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clSetKernelArg failed. (dctBuffer)"))
        return SDK_FAILURE;

    /** 
     * 4th argument to the kernel , local memory which stores intermediate values 
     */
    status = clSetKernelArg(
                    kernel, 
                    3, 
                    blockWidth * blockWidth * sizeof(cl_float), 
                    NULL);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clSetKernelArg failed. (Intermediate local buffer)"))
        return SDK_FAILURE;

    /** 
     * 5th argument to the kernel , width of the input matrix 
     */
    status = clSetKernelArg(
                    kernel, 
                    4, 
                    sizeof(cl_uint), 
                    (void *)&width);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clSetKernelArg failed. (width)"))
        return SDK_FAILURE;

    /** 
     * 6th argument to the kernel , width of the block = 8 
     */
    status = clSetKernelArg(
                    kernel, 
                    5, 
                    sizeof(cl_uint), 
                    (void *)&blockWidth);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clSetKernelArg failed. (blockWidth)"))
        return SDK_FAILURE;
    
    /**
     * 7th argument to the kernel , flag to perform inverse DCT 
     */
    status = clSetKernelArg(
                    kernel, 
                    6, 
                    sizeof(cl_uint), 
                    (void *)&inverse);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clSetKernelArg failed. (inverse)"))
        return SDK_FAILURE;

    /**
     * Enqueue a kernel run call.
     */
    status = clEnqueueNDRangeKernel(
                 commandQueue,
                 kernel,
                 2,
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


    /** 
     * wait for the kernel call to finish execution 
     */
    status = clWaitForEvents(1, &events[0]);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clWaitForEvents failed."))
        return SDK_FAILURE;

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
        totalKernelTime = (double)(kernelsEndTime - kernelsStartTime)/1e9;
    }
    clReleaseEvent(events[0]);

    /* Enqueue readBuffer*/
    status = clEnqueueReadBuffer(
                commandQueue,
                outputBuffer,
                CL_TRUE,
                0,
                width * height * sizeof(cl_float),
                output,
                0,
                NULL,
                &events[1]);
    if(!sampleCommon->checkVal(
    		status,
    		CL_SUCCESS,
    		"clEnqueueReadBuffer failed."))
    	return SDK_FAILURE;
    
    /* Wait for the read buffer to finish execution */
    status = clWaitForEvents(1, &events[1]);
    if(!sampleCommon->checkVal(
    		status,
    		CL_SUCCESS,
    		"clWaitForEvents failed."))
    	return SDK_FAILURE;

    clReleaseEvent(events[1]);
    return SDK_SUCCESS;
}



cl_uint
DCT::getIdx(cl_uint blockIdx, cl_uint blockIdy, cl_uint localIdx, cl_uint localIdy, cl_uint blockWidth, cl_uint globalWidth)
{
    cl_uint globalIdx = blockIdx * blockWidth + localIdx;
    cl_uint globalIdy = blockIdy * blockWidth + localIdy;

    return (globalIdy * globalWidth  + globalIdx);
}


/*
 * Reference implementation of the Discrete Cosine Transfrom on the CPU
 */
void 
DCT::DCTCPUReference( cl_float * verificationOutput,
                      const cl_float * input , 
                      const cl_float * dct8x8 ,
                      const cl_uint    width,
                      const cl_uint    height,
                      const cl_uint   numBlocksX,
                      const cl_uint   numBlocksY,
                      const cl_uint    inverse)
{
    cl_float * temp = (cl_float *)malloc(width*height*sizeof(cl_float));
    /* for each block in the image */
    for(cl_uint blockIdy=0; blockIdy < numBlocksY; ++blockIdy)
        for(cl_uint blockIdx=0; blockIdx < numBlocksX; ++blockIdx)
        {
            //  First calculate A^T * X
            for(cl_uint j=0; j < blockWidth ; ++j)
                for(cl_uint i = 0; i < blockWidth ; ++i)
                {
                    cl_uint index = getIdx(blockIdx, blockIdy, i, j, blockWidth, width);
                    cl_float tmp = 0.0f;
                    for(cl_uint k=0; k < blockWidth; ++k)
                    {
                        // multiply with dct8x8(k,i) if forward DCT and dct8x8(i,k) if inverse DCT
                        cl_uint index1 =  (inverse) ? i*blockWidth +k : k*blockWidth + i;
                        //input(k,j)
                        cl_uint index2 = getIdx(blockIdx, blockIdy, j, k, blockWidth, width);

                        tmp += dct8x8[index1]*input[index2];
                    }
                    temp[index] = tmp;
                }

            // And now multiply the result of previous step with A i.e. calculate (A^T * X) * A
            for(cl_uint j=0; j < blockWidth ; ++j)
                for(cl_uint i = 0; i < blockWidth ; ++i)
                {
                    cl_uint index = getIdx(blockIdx, blockIdy, i, j, blockWidth, width);
                    cl_float tmp = 0.0f;
                    for(cl_uint k=0; k < blockWidth; ++k)
                    {
                        //input(i,k)
                        cl_uint index1 = getIdx(blockIdx, blockIdy, k, i, blockWidth, width);
                        
                        // multiply with dct8x8(k,j) if forward DCT and dct8x8(j,k) if inverse DCT
                        cl_uint index2 =  (inverse) ? j*blockWidth +k : k*blockWidth + j;
                        
                        tmp += temp[index1]*dct8x8[index2];
                    }
                    verificationOutput[index] = tmp;
                }
        }
        
    free(temp);
}

int DCT::initialize()
{
    // Call base class Initialize to get default configuration
    if(!this->SDKSample::initialize())
        return SDK_FAILURE;

    streamsdk::Option* width_option = new streamsdk::Option;
    if(!width_option)
    {
        sampleCommon->error("Memory allocation error.\n");
        return SDK_FAILURE;
    }
    
    width_option->_sVersion = "x";
    width_option->_lVersion = "width";
    width_option->_description = "Width of the input matrix";
    width_option->_type = streamsdk::CA_ARG_INT;
    width_option->_value = &width;

    sampleArgs->AddOption(width_option);

    streamsdk::Option* height_option = new streamsdk::Option;
    if(!height_option)
    {
        sampleCommon->error("Memory allocation error.\n");
        return SDK_FAILURE;
    }
    
    height_option->_sVersion = "y";
    height_option->_lVersion = "height";
    height_option->_description = "Height of the input matrix";
    height_option->_type = streamsdk::CA_ARG_INT;
    height_option->_value = &height;

    sampleArgs->AddOption(height_option);

    return SDK_SUCCESS;
}

int DCT::setup()
{
    /* Make sure the width is a multiple of blockWidth 8 here */
    if(width%blockWidth != 0)
        width = (width/blockWidth + 1)*blockWidth;
    
    /* Make sure the height is a multiple of blockWidth 8 here */
    if(height%blockWidth !=0)
        height = (height/blockWidth + 1)*blockWidth;
    
    if(setupDCT()!=SDK_SUCCESS)
        return SDK_FAILURE;

    int timer = sampleCommon->createTimer();
    sampleCommon->resetTimer(timer);
    sampleCommon->startTimer(timer);

    if(setupCL()!=SDK_SUCCESS)
        return SDK_FAILURE;

    sampleCommon->stopTimer(timer);

    setupTime = (cl_double)sampleCommon->readTimer(timer);

    return SDK_SUCCESS;
}


int DCT::run()
{
    /* Arguments are set and execution call is enqueued on command buffer */
    if(runCLKernels()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(!quiet) {
        sampleCommon->printArray<cl_float>("Output", output, width,1);
    }

    return SDK_SUCCESS;
}

int DCT::verifyResults()
{
    if(verify)
    {
        verificationOutput = (cl_float *) malloc(width*height*sizeof(cl_float));
        if(verificationOutput==NULL)    { 
            sampleCommon->error("Failed to allocate host memory. (verificationOutput)");
                return SDK_FAILURE;
        }
        /**
         * reference implementation
         */
        int refTimer = sampleCommon->createTimer();
        
        sampleCommon->resetTimer(refTimer);
        sampleCommon->startTimer(refTimer);
        DCTCPUReference(verificationOutput, input, dct8x8, width, height, 
                            width/blockWidth, height/blockWidth, 0);

        sampleCommon->stopTimer(refTimer);
        referenceKernelTime = sampleCommon->readTimer(refTimer);

        /* compare the results and see if they match */

        if(sampleCommon->compare(output, verificationOutput, width*height))
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

void DCT::printStats()
{
    std::string strArray[3] = {"Width", "Height", "Time(sec)"};
    std::string stats[3];

    totalTime = setupTime + totalKernelTime;

    stats[0]  = sampleCommon->toString(width    , std::dec);
    stats[1]  = sampleCommon->toString(height   , std::dec);
    stats[2]  = sampleCommon->toString(totalTime, std::dec);
    
    this->SDKSample::printStats(strArray, stats, 3);
}

int DCT::cleanup()
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

    status = clReleaseMemObject(dctBuffer);
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
    DCT clDCT("OpenCL DCT8x8");

    clDCT.initialize();
    if(!clDCT.parseCommandLine(argc, argv))
        return SDK_FAILURE;
    if(clDCT.setup()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clDCT.run()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clDCT.verifyResults()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clDCT.cleanup()!=SDK_SUCCESS)
        return SDK_FAILURE;
    clDCT.printStats();

    return SDK_SUCCESS;
}