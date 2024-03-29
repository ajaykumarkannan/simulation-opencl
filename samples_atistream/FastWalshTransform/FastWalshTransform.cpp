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


#include "FastWalshTransform.hpp"

int 
FastWalshTransform::setupFastWalshTransform()
{
    cl_uint inputSizeBytes;

    /* allocate and init memory used by host */
    inputSizeBytes = length * sizeof(cl_float);
    input = (cl_float *) malloc(inputSizeBytes);
    if(input==NULL)    
    { 
        sampleCommon->error("Failed to allocate host memory. (input)");
        return SDK_FAILURE;
    }
    
    /* random initialisation of input */
    sampleCommon->fillRandom<cl_float>(input, length, 1, 0, 255);

    if(verify)
    {
        verificationInput = (cl_float *) malloc(inputSizeBytes);
        if(verificationInput==NULL)    { 
            sampleCommon->error("Failed to allocate host memory. (verificationInput)");
                return SDK_FAILURE;
        }
        memcpy(verificationInput, input, inputSizeBytes);
    }
    
    /* 
     * Unless quiet mode has been enabled, print the INPUT array.
     */
    if(!quiet) 
    {
        sampleCommon->printArray<cl_float>(
            "Input", 
            input, 
            length, 
            1);
    }
    
    return SDK_SUCCESS;
}

int
FastWalshTransform::setupCL(void)
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
                      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * length,
                      input, 
                      &status);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clCreateBuffer failed. (inputBuffer)"))
        return SDK_FAILURE;

    /* create a CL program using the kernel source */
    streamsdk::SDKFile kernelFile;
    kernelFile.open("FastWalshTransform_Kernels.cl");
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
    
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clCreateProgramWithBinary failed."))
        return SDK_FAILURE;

    /* create a cl program executable for all the devices specified */
    status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clBuildProgram failed."))
        return SDK_FAILURE;

    /* get a kernel object handle for a kernel with the given name */
    kernel = clCreateKernel(program, "fastWalshTransform", &status);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clCreateKernel failed."))
        return SDK_FAILURE;

    return SDK_SUCCESS;
}


int 
FastWalshTransform::runCLKernels(void)
{
    cl_int   status;
    cl_event events[2];

    size_t globalThreads[1];
    size_t localThreads[1];

    long long kernelsStartTime;
    long long kernelsEndTime;

    totalKernelTime = 0;

    /*
     * The kernel performs a butterfly operation and it runs for half the
     * total number of input elements in the array.
     * In each pass of the kernel two corresponding elements are found using
     * the butterfly operation on an array of numbers and their sum and difference
     * is stored in the same locations as the numbers
     */
    globalThreads[0] = length/2;
    localThreads[0]  = 1;

    /*** Set appropriate arguments to the kernel ***/
    /* the input array - also acts as output*/
    status = clSetKernelArg(
                    kernel, 
                    0, 
                    sizeof(cl_mem), 
                    (void *)&inputBuffer);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clSetKernelArg failed. (inputBuffer)"))
        return SDK_FAILURE;

    for(cl_int step = 1; step < length; step<<= 1) 
    {
        /* stage of the algorithm */
        status = clSetKernelArg(
                        kernel, 
                        1, 
                        sizeof(cl_int), 
                        (void *)&step);
        if(!sampleCommon->checkVal(
                status,
                CL_SUCCESS,
                "clSetKernelArg failed. (step)"))
            return SDK_FAILURE;
        
        /* Enqueue a kernel run call */
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
            /* After the final kernel call
             * store the kernel end time */
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
            totalKernelTime += (double)(kernelsEndTime - kernelsStartTime)/1e9;
        }
        
        clReleaseEvent(events[0]);
    }


    /* Enqueue readBuffer*/
    status = clEnqueueReadBuffer(
                commandQueue,
                inputBuffer,
                CL_TRUE,
                0,
                length *  sizeof(cl_float),
                input,
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

/*
 * This is the reference implementation of the FastWalsh transform
 * Here we perform the buttery operation on an array on numbers
 * to get and pair and a match indices. Their sum and differences are
 * stored in the corresponding locations and is used in the future
 * iterations to get a transformed array
 */
void 
FastWalshTransform::fastWalshTransformCPUReference(
                                cl_float * vinput, 
                                const cl_uint length)
{
    /* for each pass of the algorithm */
    for(cl_uint step=1; step < length; step <<=1)
    {
        /* length of each block */
        cl_uint jump = step << 1;
        /* for each blocks */
        for(cl_uint group = 0; group < step; ++group)
        {
            /* for each pair of elements with in the block */
            for(cl_uint pair = group; pair < length; pair += jump)
            {
                /* find its partner */
                cl_uint match = pair + step;
                
                cl_float T1 = vinput[pair];
                cl_float T2 = vinput[match];
                
                /* store the sum and difference of the numbers in the same locations */
                vinput[pair] = T1 + T2;
                vinput[match] = T1 - T2;
            }
        }
    }
}

int 
FastWalshTransform::initialize()
{
    // Call base class Initialize to get default configuration
    if(!this->SDKSample::initialize())
        return SDK_FAILURE;

    // Now add customized options
    streamsdk::Option* signal_length = new streamsdk::Option;
    if(!signal_length)
    {
        sampleCommon->error("Memory allocation error.\n");
        return SDK_FAILURE;
    }
    
    signal_length->_sVersion = "x";
    signal_length->_lVersion = "length";
    signal_length->_description = "Length of input array";
    signal_length->_type = streamsdk::CA_ARG_INT;
    signal_length->_value = &length;
    sampleArgs->AddOption(signal_length);


    return SDK_SUCCESS;
}

int 
FastWalshTransform::setup()
{
    /* make sure the length is the power of 2 */
    if(!sampleCommon->isPowerOf2(length))
        length = sampleCommon->roundToPowerOf2(length);

    if(setupFastWalshTransform()!=SDK_SUCCESS)
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


int 
FastWalshTransform::run()
{
    /* Arguments are set and execution call is enqueued on command buffer */
    if(runCLKernels()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(!quiet) {
        sampleCommon->printArray<cl_float>("Output", input, length, 1);
    }

    return SDK_SUCCESS;
}

int 
FastWalshTransform::verifyResults()
{
    if(verify) {
        /* reference implementation
         * it overwrites the input array with the output
         */
        int refTimer = sampleCommon->createTimer();
        sampleCommon->resetTimer(refTimer);
        sampleCommon->startTimer(refTimer);
        fastWalshTransformCPUReference(verificationInput, length);
        sampleCommon->stopTimer(refTimer);
        referenceKernelTime = sampleCommon->readTimer(refTimer);

        /* compare the results and see if they match */
        if(sampleCommon->compare(input, verificationInput, length))
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
FastWalshTransform::printStats()
{
    std::string strArray[2] = {"Length", "Time(sec)"};
    std::string stats[2];

    totalTime = setupTime + totalKernelTime ;

    stats[0] = sampleCommon->toString(length, std::dec);
    stats[1] = sampleCommon->toString(totalTime, std::dec);

    this->SDKSample::printStats(strArray, stats, 2);
}

int 
FastWalshTransform::cleanup()
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

    if(verificationInput) 
        free(verificationInput);

    if(devices)
        free(devices);

    return SDK_SUCCESS;
}

int 
main(int argc, char * argv[])
{
    FastWalshTransform clFastWalshTransform("OpenCL FastWalsh Transform");

    clFastWalshTransform.initialize();
    if(!clFastWalshTransform.parseCommandLine(argc, argv))
        return SDK_FAILURE;
    if(clFastWalshTransform.setup()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clFastWalshTransform.run()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clFastWalshTransform.verifyResults()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clFastWalshTransform.cleanup()!=SDK_SUCCESS)
        return SDK_FAILURE;
    clFastWalshTransform.printStats();

    return SDK_SUCCESS;
}
