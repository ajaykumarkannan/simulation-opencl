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


#include "RadixSort.hpp"

#include <math.h>

void RadixSort::hostRadixSort()
{
    cl_uint *histogram = (cl_uint*)malloc(RADICES * sizeof(cl_uint));
    if(histogram == NULL)	
        sampleCommon->error("Failed to allocate host memory. (histogram)");

    cl_uint *tempData = (cl_uint*)malloc(elementCount * sizeof(cl_uint));
    if(tempData == NULL)	
        sampleCommon->error("Failed to allocate host memory. (tempData)");

    if(histogram != NULL && tempData != NULL)
    {
        memcpy(tempData, unsortedData, elementCount * sizeof(cl_uint));

        for(int bits = 0; bits < sizeof(cl_uint) * RADIX ; bits += RADIX)
        {

            /* Initialize histogram bucket to zeros */
            memset(histogram, 0, RADICES * sizeof(cl_uint));

            /* Calculate 256 histogram for all elements*/
            for(int i = 0; i < elementCount; ++i)
            {
                cl_uint element = tempData[i];
                cl_uint value = (element >> bits) & RADIX_MASK;
                histogram[value]++;
            }

            /* Prescan the histogram bucket */
            cl_uint sum = 0;
            for(int i = 0; i < RADICES; ++i)
            {
                cl_uint val = histogram[i];
                histogram[i] = sum;
                sum += val;
            }

            /* Rearrange  the elements based on prescaned histogram */
            for(int i = 0; i < elementCount; ++i)
            {
                cl_uint element = tempData[i];
                cl_uint value = (element >> bits) & RADIX_MASK;
                cl_uint index = histogram[value];
                hSortedData[index] = tempData[i];
                histogram[value] = index + 1;
            }

            /* Copy to tempData for further use */
            if(bits != RADIX * 3)
                memcpy(tempData, hSortedData, elementCount * sizeof(cl_uint));
        }
    }

    free(tempData);
    free(histogram);
}

int RadixSort::setupRadixSort()
{
    int i = 0;

    groupSize = GROUP_SIZE;
    elementCount = sampleCommon->roundToPowerOf2<cl_uint>(elementCount);

    /* element count must be GROUP_SIZE * RADICES */
    int mulFactor = GROUP_SIZE * RADICES;

    if(elementCount < mulFactor)
        elementCount = mulFactor;
    else
        elementCount = (elementCount / mulFactor) * mulFactor;
    numGroups = elementCount / mulFactor;

    /* Allocate and init memory used by host */
    unsortedData = (cl_uint *) malloc(elementCount * sizeof(cl_uint));
    if(unsortedData == NULL)	
    { 
        sampleCommon->error("Failed to allocate host memory. (unsortedData)");
        return SDK_FAILURE;
    }

    for(i = 0; i < elementCount; i++)
    {
        unsortedData[i] = (cl_uint)rand();
    }

    dSortedData = (cl_uint *)malloc(elementCount * sizeof(cl_uint));
    if(dSortedData == NULL)	
    { 
        sampleCommon->error("Failed to allocate host memory. (dSortedData)");
        return SDK_FAILURE;
    }
    memset(dSortedData, 0, elementCount * sizeof(cl_uint));

    hSortedData = (cl_uint *)malloc(elementCount * sizeof(cl_uint));
    if(hSortedData == NULL)	
    { 
        sampleCommon->error("Failed to allocate host memory. (hSortedData)");
        return SDK_FAILURE;
    }
    memset(hSortedData, 0, elementCount * sizeof(cl_uint));

    scanedBuckets = (cl_uint *)malloc(numGroups * groupSize * RADICES * sizeof(cl_uint));
    if(scanedBuckets == NULL)	
    { 
        sampleCommon->error("Failed to allocate host memory. (scanedBuckets)");
        return SDK_FAILURE;
    }
    memset(scanedBuckets, 0, numGroups * groupSize * RADICES * sizeof(cl_uint));

    return SDK_SUCCESS;
}

int
RadixSort::setupCL(void)
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
    if(devices == NULL)
    {
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

    /* Check whether the device supports byte-addressable 
    * load/stores : required for RadixSort */
    char deviceExtensions[2048];

    /* Get list of extensions supported by the device */
    status = clGetDeviceInfo(devices[0], CL_DEVICE_EXTENSIONS, 
        sizeof(deviceExtensions), deviceExtensions, 0);

    /* Check particular extension */
    if(!strstr(deviceExtensions, "cl_khr_byte_addressable_store"))
    {
        byteRWSupport = false;
        sampleCommon->error("Device does not support sub 32bit writes!");
        return SDK_SUCCESS;
    }

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

    unsortedDataBuf = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        sizeof(cl_uint) * elementCount,
        unsortedData,
        &status);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clCreateBuffer failed. (unsortedDataBuf)"))
        return SDK_FAILURE;

    bucketsBuf = clCreateBuffer(
        context, 
        CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
        numGroups * groupSize * RADICES * sizeof(cl_uint),
        NULL, 
        &status);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clCreateBuffer failed. (bucketsBuf)"))
        return SDK_FAILURE;



    prescanedBucketsBuf = clCreateBuffer(
        context, 
        CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        numGroups * groupSize * RADICES * sizeof(cl_uint),
        scanedBuckets, 
        &status);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clCreateBuffer failed. (prescanedBucketsBuf)"))
        return SDK_FAILURE;


    sortedDataBuf = clCreateBuffer(
        context, 
        CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
        elementCount * sizeof(cl_uint),
        NULL,
        &status);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clCreateBuffer failed. (sortedDataBuf)"))
        return SDK_FAILURE;

    /* create a CL program using the kernel source */
    streamsdk::SDKFile kernelFile;
    kernelFile.open("RadixSort_Kernels.cl");
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
    histogramKernel = clCreateKernel(program, "histogram", &status);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clCreateKernel failed."))
        return SDK_FAILURE;

    permuteKernel = clCreateKernel(program, "permute", &status);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clCreateKernel failed."))
        return SDK_FAILURE;

    return SDK_SUCCESS;
}

int RadixSort::runHistogramKernel(int bits)
{
    cl_int   status;
    cl_event events[1];

    size_t globalThreads = elementCount / RADICES;
    size_t localThreads = groupSize;

	if(localThreads > maxWorkItemSizes[0] || localThreads > maxWorkGroupSize)
	{
		std::cout<<"Unsupported: Device does not support requested number of work items.";
				 
		return SDK_FAILURE;
	}

    /* Whether sort is to be in increasing order. CL_TRUE implies increasing */
    status = clSetKernelArg(histogramKernel, 0, sizeof(cl_mem), (void *)&unsortedDataBuf); 
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clSetKernelArg failed. (unsortedDataBuf)"))
        return SDK_FAILURE;

    status = clSetKernelArg(histogramKernel, 1, sizeof(cl_mem), (void *)&bucketsBuf); 
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clSetKernelArg failed. (bucketsBuf)"))
        return SDK_FAILURE;

    status = clSetKernelArg(histogramKernel, 2, sizeof(cl_int), (void *)&bits); 
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clSetKernelArg failed. (bits)"))
        return SDK_FAILURE;

    status = clSetKernelArg(histogramKernel, 3, (GROUP_SIZE * RADICES * sizeof(cl_ushort)), NULL); 
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clSetKernelArg failed. (local memory)"))
        return SDK_FAILURE;

    status = clGetKernelWorkGroupInfo(histogramKernel,
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

    /* 
    * Enqueue a kernel run call.
    */
    status = clEnqueueNDRangeKernel(
        commandQueue,
        histogramKernel,
        1,
        NULL,
        &globalThreads,
        &localThreads,
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

    /* Enqueue the results to application pointer*/
    status = clEnqueueReadBuffer(
        commandQueue, 
        bucketsBuf, 
        CL_TRUE,
        0,
        numGroups * groupSize * 256 * sizeof(cl_uint),
        scanedBuckets,
        0,
        NULL,
        &events[0]);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clEnqueueReadBuffer failed."))
        return SDK_FAILURE;


    /* wait for the read buffer to finish execution */
    status = clWaitForEvents(1, &events[0]);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clWaitForEvents failed."))
        return SDK_FAILURE;

    clReleaseEvent(events[0]);

    return SDK_SUCCESS;
}


int
RadixSort::runPermuteKernel(int bits)
{
    cl_int   status;
    cl_event events[1];

    size_t globalThreads = elementCount / RADICES;
    size_t localThreads = groupSize;

	if(localThreads > maxWorkItemSizes[0] || localThreads > maxWorkGroupSize)
	{
		std::cout<<"Unsupported: Device does not support requested number of work items.";
				 
		return SDK_FAILURE;
	}

    /* Whether sort is to be in increasing order. CL_TRUE implies increasing */
    status = clSetKernelArg(permuteKernel, 0, sizeof(cl_mem), (void *)&unsortedDataBuf); 
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clSetKernelArg failed. (unsortedDataBuf)"))
        return SDK_FAILURE;

    status = clSetKernelArg(permuteKernel, 1, sizeof(cl_mem), (void *)&prescanedBucketsBuf); 
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clSetKernelArg failed. (prescanedBucketsBuf)"))
        return SDK_FAILURE;


    status = clSetKernelArg(permuteKernel, 2, sizeof(cl_int), (void *)&bits); 
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clSetKernelArg failed. (bits)"))
        return SDK_FAILURE;

    status = clSetKernelArg(permuteKernel, 3, (GROUP_SIZE * RADICES * sizeof(cl_ushort)), NULL); 
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clSetKernelArg failed. (local memory)"))
        return SDK_FAILURE;

    status = clSetKernelArg(permuteKernel, 4, sizeof(cl_mem), (void *)&sortedDataBuf); 
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clSetKernelArg failed. (sortedDataBuf)"))
        return SDK_FAILURE;

    status = clGetKernelWorkGroupInfo(permuteKernel,
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

    /* 
    * Enqueue a kernel run call.
    */
    status = clEnqueueNDRangeKernel(
        commandQueue,
        permuteKernel,
        1,
        NULL,
        &globalThreads,
        &localThreads,
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

    /* Enqueue the results to application pointer*/
    status = clEnqueueReadBuffer(
        commandQueue, 
        sortedDataBuf, 
        CL_TRUE,
        0,
        elementCount * sizeof(cl_uint),
        dSortedData,
        0,
        NULL,
        &events[0]);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clEnqueueReadBuffer failed."))
        return SDK_FAILURE;

    /* wait for the read buffer to finish execution */
    status = clWaitForEvents(1, &events[0]);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clWaitForEvents failed."))
        return SDK_FAILURE;

    clReleaseEvent(events[0]);

    return SDK_SUCCESS;
}
int 
RadixSort::runCLKernels(void)
{

    for(int bits = 0; bits < sizeof(cl_uint) * RADIX; bits += RADIX)
    {
        /* Calculate thread-histograms */
        runHistogramKernel(bits);



        /* Prescan the histogram buckets */
        int sum = 0;
        for(int i = 0; i < RADICES; ++i)
        {
            for(int j = 0; j < numGroups; ++j)
            {
                for(int k = 0; k < groupSize; ++k)
                {
                    int index = j * groupSize * RADICES + k * RADICES + i;
                    int value = scanedBuckets[index];
                    scanedBuckets[index] = sum;
                    sum += value;
                }
            }
        }


        /* Permute the element to appropriate place */
        runPermuteKernel(bits);

        memcpy(unsortedData, dSortedData, elementCount * sizeof(cl_uint));
    }

    return SDK_SUCCESS;
}

int RadixSort::initialize()
{
    // Call base class Initialize to get default configuration
    if(!this->SDKSample::initialize())
        return SDK_FAILURE;

    // Now add customized options
    streamsdk::Option* array_length = new streamsdk::Option;
    if(!array_length)
    {
        sampleCommon->error("Memory allocation error.\n");
        return SDK_FAILURE;
    }

    array_length->_sVersion = "x";
    array_length->_lVersion = "count";
    array_length->_description = "Element count";
    array_length->_type = streamsdk::CA_ARG_INT;
    array_length->_value = &elementCount;
    sampleArgs->AddOption(array_length);

    return SDK_SUCCESS;
}

int RadixSort::setup()
{
    if(setupRadixSort()!=SDK_SUCCESS)
        return SDK_FAILURE;

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


int RadixSort::run()
{
    if(!byteRWSupport)
        return SDK_SUCCESS;

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
        sampleCommon->printArray<cl_uint>("dSortedData", dSortedData, 512, 1);

    return SDK_SUCCESS;
}

int RadixSort::verifyResults()
{
    if(!byteRWSupport)
        return SDK_SUCCESS;

    if(verify)
    {
        /* Rreference implementation on host device
        * Sorted by radix sort
        */
        hostRadixSort();

        /* compare the results and see if they match */
        bool result = true;
        for(int i = 0; i < elementCount; ++i)
        {
            if(dSortedData[i] != hSortedData[i])
            {
                result = false;
                break;
            }
        }

        if(result)
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

void RadixSort::printStats()
{
    std::string strArray[2] = {"Elements", "Time(sec)"};
    std::string stats[2];

    totalTime = setupTime + kernelTime;
    stats[0]  = sampleCommon->toString(elementCount, std::dec);
    stats[1]  = sampleCommon->toString(totalTime, std::dec);

    this->SDKSample::printStats(strArray, stats, 2);

}


int RadixSort::cleanup()
{
    if(!byteRWSupport)
        return SDK_SUCCESS;

    /* Releases OpenCL resources (Context, Memory etc.) */
    cl_int status;

    status = clReleaseMemObject(unsortedDataBuf);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseMemObject failed."))
        return SDK_FAILURE;

    status = clReleaseMemObject(sortedDataBuf);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseMemObject failed."))
        return SDK_FAILURE;

    status = clReleaseMemObject(bucketsBuf);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseMemObject failed."))
        return SDK_FAILURE;

    status = clReleaseMemObject(prescanedBucketsBuf);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseMemObject failed."))
        return SDK_FAILURE;

    status = clReleaseKernel(histogramKernel);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseKernel failed."))
        return SDK_FAILURE;

    status = clReleaseKernel(permuteKernel);
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

    /* Release program resources (input memory etc.) */
    if(unsortedData) 
    {
        free(unsortedData);
        unsortedData = NULL;
    }

    if(dSortedData) 
    {
        free(dSortedData);
        dSortedData = NULL;
    }

    if(hSortedData) 
    {
        free(hSortedData);
        hSortedData = NULL;
    }

    if(scanedBuckets) 
    {
        free(scanedBuckets);
        scanedBuckets = NULL;
    }

    if(devices)
    {
        free(devices);
        devices = NULL;
    }

    if(maxWorkItemSizes)
    {
        free(maxWorkItemSizes);
        maxWorkItemSizes = NULL;
    }
    
    return SDK_SUCCESS;
}

int 
main(int argc, char * argv[])
{
    /* Create MonteCalroAsian object */
    RadixSort clRadixSort("RadixSort sample");

    /* Initialization */
    if(clRadixSort.initialize()!=SDK_SUCCESS)
		return SDK_FAILURE;

    /* Parse command line options */
    if(!clRadixSort.parseCommandLine(argc, argv))
        return SDK_FAILURE;

    /* Setup */
    if(clRadixSort.setup()!=SDK_SUCCESS)
        return SDK_FAILURE;

    /* Run */
    if(clRadixSort.run()!=SDK_SUCCESS)
		return SDK_FAILURE;

    /* Verify */
    if(clRadixSort.verifyResults()!=SDK_SUCCESS)
		return SDK_FAILURE;

    /* Cleanup resources created */
	if(clRadixSort.cleanup()!=SDK_SUCCESS)
		return SDK_FAILURE;

    /* Print performance statistics */
    clRadixSort.printStats();

    return SDK_SUCCESS;
}
