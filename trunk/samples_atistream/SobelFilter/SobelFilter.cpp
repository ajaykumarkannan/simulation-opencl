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


#include "SobelFilter.hpp"
#include <cmath>


int
SobelFilter::readInputImage(std::string inputImageName)
{
    
    /* load input bitmap image */
    inputBitmap.load(inputImageName.c_str());

    /* error if image did not load */
    if(!inputBitmap.isLoaded())
    {
        sampleCommon->error("Failed to load input image!");
        return SDK_FAILURE;
    }


    /* get width and height of input image */
    height = inputBitmap.getHeight();
    width = inputBitmap.getWidth();


    /* allocate memory for input & output image data  */
    inputImageData  = (cl_uchar*)malloc(width * height * pixelSize);

    /* error check */
    if(inputImageData == NULL)
    {
        sampleCommon->error("Failed to allocate memory! (inputImageData)");
        return SDK_FAILURE;
    }


    /* allocate memory for output image data */
    outputImageData = (cl_uchar*)malloc(width * height * pixelSize);


    /* error check */
    if(outputImageData == NULL)
    {
        sampleCommon->error("Failed to allocate memory! (outputImageData)");
        return SDK_FAILURE;
    }

    /* initializa the Image data to NULL */
    memset(outputImageData, 0, width * height * pixelSize);

    /* get the pointer to pixel data */
    pixelData = inputBitmap.getPixels();


    /* error check */
    if(pixelData == NULL)
    {
        sampleCommon->error("Failed to read pixel Data!");
        return SDK_FAILURE;
    }


    /* Copy pixel data into inputImageData */
    memcpy(inputImageData, pixelData, width * height * pixelSize);


    /* allocate memory for verification output */
    verificationOutput = (cl_uchar*)malloc(width * height * pixelSize);

    /* error check */
    if(verificationOutput == NULL)
    {
        sampleCommon->error("verificationOutput heap allocation failed!");
        return SDK_FAILURE;
    }

    /* initialize the data to NULL */
    memset(verificationOutput, 0, width * height * pixelSize);

    return SDK_SUCCESS;

}


int
SobelFilter::writeOutputImage(std::string outputImageName)
{
    /* copy output image data back to original pixel data */
    memcpy(pixelData, outputImageData, width * height * pixelSize);

    /* write the output bmp file */
    if(!inputBitmap.write(outputImageName.c_str()))
    {
        sampleCommon->error("Failed to write output image!");
        return SDK_FAILURE;
    }

    return SDK_SUCCESS;
}
    


int 
SobelFilter::setupCL()
{

    cl_int status = CL_SUCCESS;

    cl_device_type dType;
    
    if(deviceType.compare("cpu") == 0)
    {
        dType = CL_DEVICE_TYPE_CPU;
    }
    else //deviceType = "gpu" 
    {
        dType = CL_DEVICE_TYPE_GPU;
    }

    size_t deviceListSize;

    /* Create context from given device type */
    context = clCreateContextFromType(0,
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
    {
        return SDK_FAILURE;
    }

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

    /* Check whether the device supports byte-addressable 
     * load/stores : required for SobelFilter */
    char deviceExtensions[2048];
    
    status = clGetDeviceInfo(devices[0], CL_DEVICE_EXTENSIONS, 
                    sizeof(deviceExtensions), deviceExtensions, 0);

    if(!strstr(deviceExtensions, "cl_khr_byte_addressable_store"))
    {
        byteRWSupport = false;
        sampleCommon->error("Device does not support sub 32bit writes!");
        return SDK_SUCCESS;
    }


    /* Create command queue */

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
                        CL_SUCCESS,
                        "clCreateCommandQueue failed."))
    {
        return SDK_FAILURE;
    }

    /*
     * Create and initialize memory objects
     */

    /* Create memory object for input Image */
    inputImageBuffer = clCreateBuffer(
                      context,
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      width * height * pixelSize,
                      inputImageData,
                      &status);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS,
                        "clCreateBuffer failed. (inputImageBuffer)"))
    {
        return SDK_FAILURE;
    }

    /* Create memory objects for output Image */
    outputImageBuffer = clCreateBuffer(context,
                                  CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                  width * height * pixelSize,
                                  outputImageData,
                                  &status);
    if(!sampleCommon->checkVal(status,
                               CL_SUCCESS,
                               "clCreateBuffer failed. (outputImageBuffer)"))
    {
        return SDK_FAILURE;
    }

  

    /* create a CL program using the kernel source */
    streamsdk::SDKFile kernelFile;
    kernelFile.open("SobelFilter_Kernels.cl");
    const char * source = kernelFile.source().c_str();
    size_t sourceSize[] = { strlen(source) };
    program = clCreateProgramWithSource(context,
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
    status = clBuildProgram(
                 program,
                 1,
                 devices,
                 NULL,
                 NULL,
                 NULL);
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
    kernel = clCreateKernel(
                 program,
                 "sobel_filter",
                 &status);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS,
                        "clCreateKernel failed."))
    {
        return SDK_FAILURE;
    }

    return SDK_SUCCESS;
}

int 
SobelFilter::runCLKernels()
{
    cl_int status;
    cl_event events[2];

    long long kernelsStartTime;
    long long kernelsEndTime;

    /*** Set appropriate arguments to the kernel ***/

    /* input buffer image */
    status = clSetKernelArg(
                 kernel,
                 0,
                 sizeof(cl_mem),
                 &inputImageBuffer);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clSetKernelArg failed. (inputImageBuffer)"))
    {
        return SDK_FAILURE;
    }

    /* outBuffer imager */
    status = clSetKernelArg(
                 kernel,
                 1,
                 sizeof(cl_mem),
                 &outputImageBuffer);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clSetKernelArg failed. (outputImageBuffer)"))
    {
        return SDK_FAILURE;
    }

    

    /* 
     * Enqueue a kernel run call.
     */
    size_t globalThreads[] = {width * 4, height};

    status = clEnqueueNDRangeKernel(
                 commandQueue,
                 kernel,
                 2,
                 NULL,
                 globalThreads,
                 0,
                 0,
                 NULL,
                 &events[0]);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clEnqueueNDRangeKernel failed."))
    {
        return SDK_FAILURE;
    }

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
                "clGetEventProfilingInfo1 failed."))
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
                "clGetEventProfilingInfo2 failed."))
            return SDK_FAILURE;

        /* Compute total time (also convert from nanoseconds to seconds) */
        kernelTime = (double)(kernelsEndTime - kernelsStartTime)/1e9;
    }


    clReleaseEvent(events[0]);

    /* Enqueue readBuffer*/
    status = clEnqueueReadBuffer(
                commandQueue,
                outputImageBuffer,
                CL_TRUE,
                0,
                width * height * pixelSize,
                outputImageData,
                0,
                NULL,
                &events[0]);
    if(!sampleCommon->checkVal(
    		status,
    		CL_SUCCESS,
    		"clEnqueueReadBuffer failed."))
    	return SDK_FAILURE;
    
    /* Wait for the read buffer to finish execution */
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
SobelFilter::initialize()
{
    // Call base class Initialize to get default configuration
    if(!this->SDKSample::initialize())
        return SDK_FAILURE;

    return SDK_SUCCESS;
}

int 
SobelFilter::setup()
{
    /* Allocate host memory and read input image */
    if(readInputImage(INPUT_IMAGE)!=SDK_SUCCESS)
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

int 
SobelFilter::run()
{
    if(!byteRWSupport)
        return SDK_SUCCESS;

    /* Set kernel arguments and run kernel */
    if(runCLKernels()!=SDK_SUCCESS)
        return SDK_FAILURE;

    /* write the output image to bitmap file */
    if(writeOutputImage(OUTPUT_IMAGE) != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    return SDK_SUCCESS;
}

int 
SobelFilter::cleanup()
{
    if(!byteRWSupport)
        return SDK_SUCCESS;

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

    status = clReleaseMemObject(inputImageBuffer);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseMemObject failed."))
        return SDK_FAILURE;

    status = clReleaseMemObject(outputImageBuffer);
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
    if(inputImageData) 
        free(inputImageData);

    if(outputImageData)
        free(outputImageData);

    if(verificationOutput) 
        free(verificationOutput);

    if(devices)
        free(devices);

    return SDK_SUCCESS;
}


void 
SobelFilter::sobelFilterCPUReference()
{
    /* x-axis gradient mask */
    const int kx[][3] = { { 1, 2, 1},
	                      {  0, 0, 0},
	                      { -1,-2,-1}} ;

    /* y-axis gradient mask */
    const int ky[][3] = { { 1, 0, -1},
	                      { 2, 0, -2},
	                      { 1, 0, -1}};

    int gx = 0;
    int gy = 0;

    /* pointer to input image data */
    cl_uchar *ptr = inputImageData;

    /* each pixel has 4 uchar components */
    int w = width * 4;

    int k = 1;

    /* apply filter on each pixel (except boundary pixels) */
    for(int i=0; i < (int)(w * (height - 1)) ; i++) 
    {
        if( i < (k+1)*w - 4 && i >= 4 + k*w )
        {
            gx =  kx[0][0] * *(ptr + i - 4 - w)  
                + kx[0][1] * *(ptr + i - w) 
                + kx[0][2] * *(ptr + i + 4 - w)
                + kx[1][0] * *(ptr + i - 4)     
                + kx[1][1] * *(ptr + i)      
                + kx[1][2] * *(ptr + i + 4)
                + kx[2][0] * *(ptr + i - 4 + w) 
                + kx[2][1] * *(ptr + i + w) 
                + kx[2][2] * *(ptr + i + 4 + w);


            gy =  ky[0][0] * *(ptr + i - 4 - w) 
                + ky[0][1] * *(ptr + i - w) 
                + ky[0][2] * *(ptr + i + 4 - w)
                + ky[1][0] * *(ptr + i - 4)     
                + ky[1][1] * *(ptr + i)      
                + ky[1][2] * *(ptr + i + 4)
                + ky[2][0] * *(ptr + i - 4 + w) 
                + ky[2][1] * *(ptr + i + w) 
                + ky[2][2] * *(ptr + i + 4 + w);

            float gx2 = pow((float)gx, 2);
            float gy2 = pow((float)gy, 2);

           
            *(verificationOutput + i) = (cl_uchar)(sqrt(gx2 + gy2) / 2.0);       
        }

        /* if reached at the end of its row then incr k */
        if( i == (k+1)*w - 5 )
        {
            k++;
        }    
    }
}


int 
SobelFilter::verifyResults()
{
    if(!byteRWSupport)
        return SDK_SUCCESS;
        
    if(verify)
    {
        /* reference implementation */
        sobelFilterCPUReference();

        float *outputDevice = new float[width * height * pixelSize];
        if(outputDevice == NULL)
        {
            sampleCommon->error("Failed to allocate host memory! (outputDevice)");
            return SDK_FAILURE;
        }

        float *outputReference = new float[width * height * pixelSize];
        if(outputReference == NULL)
        {
            sampleCommon->error("Failed to allocate host memory! (outputReference)");
            return SDK_FAILURE;
        }

        /* copy uchar data to float array */
        for(int i=0; i < (int)(width * height * pixelSize); i++)
        {
            outputDevice[i] = outputImageData[i];
            outputReference[i] = verificationOutput[i];
        }

        /* compare the results and see if they match */
        if(sampleCommon->compare(outputReference, outputDevice, width*height*4))
        {
            std::cout<<"Passed!\n";
            delete[] outputDevice;
            delete[] outputReference;
            return SDK_SUCCESS;
        }
        else
        {
            std::cout<<"Failed\n";
            delete[] outputDevice;
            delete[] outputReference;
            return SDK_FAILURE;
        }
    }

	return SDK_SUCCESS;
}

void 
SobelFilter::printStats()
{
    std::string strArray[3] = {"Width", "Height", "Time(sec)"};
    std::string stats[3];
    
    totalTime = setupTime + kernelTime;

    stats[0]  = sampleCommon->toString(width, std::dec);
    stats[1]  = sampleCommon->toString(height, std::dec);
    stats[2]  = sampleCommon->toString(totalTime, std::dec);
    
    this->SDKSample::printStats(strArray, stats, 3);
}


int 
main(int argc, char * argv[])
{
    SobelFilter clSobelFilter("OpenCL SobelFilter");
    
    if(clSobelFilter.initialize()!= SDK_SUCCESS)
        return SDK_FAILURE;

    if(!clSobelFilter.parseCommandLine(argc, argv))
        return SDK_FAILURE;

    if(clSobelFilter.setup()!= SDK_SUCCESS)
        return SDK_FAILURE;

    if(clSobelFilter.run()!= SDK_SUCCESS)
        return SDK_FAILURE;

    if(clSobelFilter.verifyResults()!= SDK_SUCCESS)
        return SDK_FAILURE;
    if(clSobelFilter.cleanup()!= SDK_SUCCESS)
        return SDK_FAILURE;

    clSobelFilter.printStats();

    return SDK_SUCCESS;
}



