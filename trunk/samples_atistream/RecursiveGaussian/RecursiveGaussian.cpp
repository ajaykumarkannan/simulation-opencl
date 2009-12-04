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


#include "RecursiveGaussian.hpp"
#include <cmath>


#define BLOCK_DIM 8        // Thread block dim for transpose kernel
#define GROUP_SIZE 64		// Group size for RecursiveGaussian kernel

int
RecursiveGaussian::readInputImage(std::string inputImageName)
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

	/* Check width and height against GROUP_SIZE */
	if( width % GROUP_SIZE != 0 || height % GROUP_SIZE != 0 )
	{
		sampleCommon->error("Width(and Height) should be a multiple of GROUP_SIZE!");
		return SDK_FAILURE;
	}

	/* Check width and height against BLOCK_DIM */
	if( width % BLOCK_DIM !=0 || height % BLOCK_DIM !=0 )
	{
		sampleCommon->error("Width(and Height) should be a multiple of BLOCK_DIM!");
		return SDK_FAILURE;
	}

    /* allocate memory for input & output image data  */
    inputImageData  = (cl_uchar4*)malloc(width * height * sizeof(cl_uchar4));

    /* error check */
    if(inputImageData == NULL)
    {
        sampleCommon->error("Failed to allocate memory! (inputImageData)");
        return SDK_FAILURE;
    }

    /* allocate memory for output image data */
    outputImageData = (cl_uchar4*)malloc(width * height * sizeof(cl_uchar4));

    /* error check */
    if(outputImageData == NULL)
    {
        sampleCommon->error("Failed to allocate memory! (outputImageData)");
        return SDK_FAILURE;
    }


    /* initializa the Image data to NULL */
    memset(outputImageData, 0, width * height * sizeof(cl_uchar4));
    
    /* get the pointer to pixel data */
    pixelData = inputBitmap.getPixels();

    /* error check */
    if(pixelData == NULL)
    {
        sampleCommon->error("Failed to read pixel Data!");
        return SDK_FAILURE;
    }

    /* Copy pixel data into inputImageData */
    memcpy(inputImageData, pixelData, width * height * sizeof(cl_uchar4));

    /* allocate memory for verification output */
    verificationOutput = (cl_uchar4*)malloc(width * height * sizeof(cl_uchar4));
    
    /* error check */
    if(verificationOutput == NULL)
    {
        sampleCommon->error("Failed to allocate memory! (verificationOutput)");
        return SDK_FAILURE;
    }

    /* initialize the data to NULL */
    memset(verificationOutput, 0, width * height * sizeof(cl_uchar4));

    return SDK_SUCCESS;

}


int
RecursiveGaussian::writeOutputImage(std::string outputImageName)
{
    /* copy output image data back to original pixel data */
    memcpy(pixelData, outputImageData, width * height * sizeof(cl_uchar4));

    /* write the output bmp file */
    if(!inputBitmap.write(outputImageName.c_str()))
    {
        sampleCommon->error("Failed to write output image!");
        return SDK_FAILURE;
    }

    return SDK_SUCCESS;
}


void
RecursiveGaussian::computeGaussParms(float fSigma, int iOrder, GaussParms* pGP)
{
    // pre-compute filter coefficients
    pGP->nsigma = fSigma; // note: fSigma is range-checked and clamped >= 0.1f upstream
    pGP->alpha = 1.695f / pGP->nsigma;
    pGP->ema = exp(-pGP->alpha);
    pGP->ema2 = exp(-2.0f * pGP->alpha);
    pGP->b1 = -2.0f * pGP->ema;
    pGP->b2 = pGP->ema2;
    pGP->a0 = 0.0f;
    pGP->a1 = 0.0f;
    pGP->a2 = 0.0f;
    pGP->a3 = 0.0f;
    pGP->coefp = 0.0f;
    pGP->coefn = 0.0f;

    switch (iOrder) 
    {
        case 0: 
            {
                const float k = (1.0f - pGP->ema)*(1.0f - pGP->ema)/(1.0f + (2.0f * pGP->alpha * pGP->ema) - pGP->ema2);
                pGP->a0 = k;
                pGP->a1 = k * (pGP->alpha - 1.0f) * pGP->ema;
                pGP->a2 = k * (pGP->alpha + 1.0f) * pGP->ema;
                pGP->a3 = -k * pGP->ema2;
            } 
            break;
        case 1: 
            {
                pGP->a0 = (1.0f - pGP->ema) * (1.0f - pGP->ema);
                pGP->a1 = 0.0f;
                pGP->a2 = -pGP->a0;
                pGP->a3 = 0.0f;
            } 
            break;
        case 2: 
            {
                const float ea = exp(-pGP->alpha);
                const float k = -(pGP->ema2 - 1.0f)/(2.0f * pGP->alpha * pGP->ema);
                float kn = -2.0f * (-1.0f + (3.0f * ea) - (3.0f * ea * ea) + (ea * ea * ea));
                kn /= (((3.0f * ea) + 1.0f + (3.0f * ea * ea) + (ea * ea * ea)));
                pGP->a0 = kn;
                pGP->a1 = -kn * (1.0f + (k * pGP->alpha)) * pGP->ema;
                pGP->a2 = kn * (1.0f - (k * pGP->alpha)) * pGP->ema;
                pGP->a3 = -kn * pGP->ema2;
            } 
            break;
        default:
            // note: iOrder is range-checked and clamped to 0-2 upstream
            return;
    }
    pGP->coefp = (pGP->a0 + pGP->a1)/(1.0f + pGP->b1 + pGP->b2);
    pGP->coefn = (pGP->a2 + pGP->a3)/(1.0f + pGP->b1 + pGP->b2);
}


    

int 
RecursiveGaussian::setupCL()
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


    /**
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

    /* create memory object for temp buffer */
    tempImageBuffer = clCreateBuffer(context,
                                     CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                     width * height * pixelSize,
                                     0,
                                     &status);

    if(!sampleCommon->checkVal(status,
                               CL_SUCCESS,
                               "clCreateBuffer failed. (tempImageBuffer)"))
    {
        return SDK_FAILURE;
    }

  

    /* create a CL program using the kernel source */
    streamsdk::SDKFile kernelFile;
    kernelFile.open("RecursiveGaussian_Kernels.cl");
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

    /* kernel object for transpose kernel */
    kernelTranspose = clCreateKernel(program,
                                     "transpose_kernel",
                                      &status);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS,
                        "clCreateKernel failed.(transpose_kernel)"))
    {
        return SDK_FAILURE;
    }

    /* kernel object for recursive gaussian kernel */
    kernelRecursiveGaussian = clCreateKernel(program,
                                             "RecursiveGaussian_kernel",
                                             &status);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS,
                        "clCreateKernel failed.(RecursiveGaussian_kernel)"))
    {
        return SDK_FAILURE;
    }

    return SDK_SUCCESS;
}

int 
RecursiveGaussian::runCLKernels()
{

    cl_int status;
    cl_event events[2];

    /* initialize Gaussian parameters */ 
    float fSigma = 10.0f;               // filter sigma (blur factor)
    int iOrder = 0;                     // filter order

    /* compute gaussian parameters */
    computeGaussParms(fSigma, iOrder, &oclGP);

    long long kernelsStartTime;
    long long kernelsEndTime;

    /*** Set appropriate arguments to the kernel (Recursive Gaussian) ***/

    /* input : input buffer image */
    status = clSetKernelArg(
                 kernelRecursiveGaussian,
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

    /* output : temp Buffer */
    status = clSetKernelArg(
                 kernelRecursiveGaussian,
                 1,
                 sizeof(cl_mem),
                 &tempImageBuffer);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clSetKernelArg failed. (outputImageBuffer)"))
    {
        return SDK_FAILURE;
    }

    /* image width */ 
    status = clSetKernelArg(kernelRecursiveGaussian,
                            2,
                            sizeof(cl_int),
                            &width);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (width)"))
    {
        return SDK_FAILURE;
    }

    /* image height */ 
    status = clSetKernelArg(kernelRecursiveGaussian,
                            3,
                            sizeof(cl_int),
                            &height);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (height)"))
    {
        return SDK_FAILURE;
    }

    /* gaussian parameter : a0 */ 
    status = clSetKernelArg(kernelRecursiveGaussian,
                            4,
                            sizeof(cl_float),
                            &oclGP.a0);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (oclGP.a0)"))
    {
        return SDK_FAILURE;
    }

    /* gaussian parameter : a1 */
    status = clSetKernelArg(kernelRecursiveGaussian,
                            5,
                            sizeof(cl_float),
                            &oclGP.a1);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (oclGP.a1)"))
    {
        return SDK_FAILURE;
    }


    /* gaussian parameter : a2 */
    status = clSetKernelArg(kernelRecursiveGaussian,
                            6,
                            sizeof(cl_float),
                            &oclGP.a2);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (oclGP.a2)"))
    {
        return SDK_FAILURE;
    }

    /* gaussian parameter : a3 */
    status = clSetKernelArg(kernelRecursiveGaussian,
                            7,
                            sizeof(cl_float),
                            &oclGP.a3);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (oclGP.a3)"))
    {
        return SDK_FAILURE;
    }

    /* gaussian parameter : b1 */
    status = clSetKernelArg(kernelRecursiveGaussian,
                            8,
                            sizeof(cl_float),
                            &oclGP.b1);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (oclGP.b1)"))
    {
        return SDK_FAILURE;
    }

    /* gaussian parameter : b2 */
    status = clSetKernelArg(kernelRecursiveGaussian,
                            9,
                            sizeof(cl_float),
                            &oclGP.b2);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (oclGP.b2)"))
    {
        return SDK_FAILURE;
    }

    /* gaussian parameter : coefp */
    status = clSetKernelArg(kernelRecursiveGaussian,
                            10,
                            sizeof(cl_float),
                            &oclGP.coefp);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (oclGP.coefp)"))
    {
        return SDK_FAILURE;
    }

    /* gaussian parameter : coefn */
    status = clSetKernelArg(kernelRecursiveGaussian,
                            11,
                            sizeof(cl_float),
                            &oclGP.coefn);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (oclGP.coefn)"))
    {
        return SDK_FAILURE;
    }
   


    /* set global index and group size */
    size_t globalThreads[] = {width, 1};
	size_t localThreads[] = {GROUP_SIZE, 1};

	if(localThreads[0] > maxWorkItemSizes[0] || localThreads[0] > maxWorkGroupSize)
	{
		std::cout<<"Unsupported: Device does not support requested number of work items.";
				 
		return SDK_FAILURE;
	}
    
    /* 
     * Enqueue a kernel run call.
     */
    status = clEnqueueNDRangeKernel(
                 commandQueue,
                 kernelRecursiveGaussian,
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
    {
        return SDK_FAILURE;
    }

    /* Wait for kernel to finish */
    status = clWaitForEvents(1, &events[0]);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clWaitForEvents failed."))
        return SDK_FAILURE;


    /*** Set appropriate arguments to the kernel (Transpose) ***/

    /* output : input buffer image  */
    status = clSetKernelArg(
                 kernelTranspose,
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

    /* input : temp Buffer */
    status = clSetKernelArg(
                 kernelTranspose,
                 1,
                 sizeof(cl_mem),
                 &tempImageBuffer);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clSetKernelArg failed. (tempImageBuffer)"))
    {
        return SDK_FAILURE;
    }

    /* local memory for block transpose */ 
    status = clSetKernelArg(kernelTranspose,
                            2,
                            BLOCK_DIM * BLOCK_DIM * sizeof(cl_uchar4),
                            NULL);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (local)"))
    {
        return SDK_FAILURE;
    }

    /* image width */ 
    status = clSetKernelArg(kernelTranspose,
                            3,
                            sizeof(cl_int),
                            &width);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (width)"))
    {
        return SDK_FAILURE;
    }

    /* image height */ 
    status = clSetKernelArg(kernelTranspose,
                            4,
                            sizeof(cl_int),
                            &height);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (height)"))
    {
        return SDK_FAILURE;
    }

    /* size of block (group) */
    int blockSize = BLOCK_DIM;

    /* block_size */
    status = clSetKernelArg(kernelTranspose,
                            5,
                            sizeof(cl_int),
                            &blockSize);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (blockSize)"))
    {
        return SDK_FAILURE;
    }

    /* group dimensions for transpose kernel */
    size_t localThreadsT[] = {BLOCK_DIM, BLOCK_DIM};

    size_t globalThreadsT[] = {width, height};


	if(localThreadsT[0] > maxWorkItemSizes[0] 
        || localThreadsT[1] > maxWorkItemSizes[1]
        || localThreadsT[0] * localThreadsT[1] > maxWorkGroupSize)
	{
		std::cout<<"Unsupported: Device does not support requested number of work items.";
				 
		return SDK_FAILURE;
	}

    status = clGetKernelWorkGroupInfo(kernelTranspose,
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

    /* Enqueue Transpose Kernel */
    status = clEnqueueNDRangeKernel(
                 commandQueue,
                 kernelTranspose,
                 2,
                 NULL,
                 globalThreadsT,
                 localThreadsT,
                 0,
                 NULL,
                 &events[1]);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clEnqueueNDRangeKernel failed."))

    /* Wait for transpose Kernel to finish */
    status = clWaitForEvents(1, &events[0]);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clWaitForEvents failed."))
        return SDK_FAILURE;


    /* Set Arguments for Recursive Gaussian Kernel 
        Image is now transposed  
        new_width = height
        new_height = width */


    /* image width : swap with height */ 
    status = clSetKernelArg(kernelRecursiveGaussian, 2, sizeof(cl_int), &height);

    if(!sampleCommon->checkVal(status, 
                               CL_SUCCESS,
                               "clSetKernelArg Failed. (height)"))
    {
        return SDK_FAILURE;
    }

    /* image height */ 
    status = clSetKernelArg(kernelRecursiveGaussian, 3, sizeof(cl_int), &width);

    if(!sampleCommon->checkVal(status, 
                               CL_SUCCESS,
                               "clSetKernelArg Failed. (width)"))
    {
        return SDK_FAILURE;
    }


    /* Set new global index */
    globalThreads[0] = height;
    globalThreads[1] = 1;

    status = clEnqueueNDRangeKernel(
                 commandQueue,
                 kernelRecursiveGaussian,
                 2,
                 NULL,
                 globalThreads,
                 localThreads,
                 0,
                 NULL,
                 &events[1]);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clEnqueueNDRangeKernel failed."))
    {
        return SDK_FAILURE;
    }

    /* Wait for Recursive Gaussian Kernel to finish */
    status = clWaitForEvents(1, &events[1]);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clWaitForEvents failed."))
        return SDK_FAILURE;


    /* Set Arguments to Transpose Kernel */

    /* output : output buffer image  */
    status = clSetKernelArg(
                 kernelTranspose,
                 0,
                 sizeof(cl_mem),
                 &outputImageBuffer);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clSetKernelArg failed. (outputImageBuffer)"))
    {
        return SDK_FAILURE;
    }

    /* input : temp Buffer */
    status = clSetKernelArg(
                 kernelTranspose,
                 1,
                 sizeof(cl_mem),
                 &tempImageBuffer);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clSetKernelArg failed. (tempImageBuffer)"))
    {
        return SDK_FAILURE;
    }

    /* local memory for block transpose */ 
    status = clSetKernelArg(kernelTranspose,
                            2,
                            BLOCK_DIM * BLOCK_DIM * sizeof(cl_uchar4),
                            NULL);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (local)"))
    {
        return SDK_FAILURE;
    }

    /* image width : is height actually as the image is currently transposed*/ 
    status = clSetKernelArg(kernelTranspose,
                            3,
                            sizeof(cl_int),
                            &height);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (height)"))
    {
        return SDK_FAILURE;
    }

    /* image height */ 
    status = clSetKernelArg(kernelTranspose,
                            4,
                            sizeof(cl_int),
                            &width);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (width)"))
    {
        return SDK_FAILURE;
    }


    /* block_size */
    status = clSetKernelArg(kernelTranspose,
                            5,
                            sizeof(cl_int),
                            &blockSize);
    if(!sampleCommon->checkVal(status, CL_SUCCESS,
                                "clSetKernelArg Failed. (blockSize)"))
    {
        return SDK_FAILURE;
    }

    /* group dimensions for transpose kernel */
    globalThreadsT[0] = height;
    globalThreadsT[1] = width;

    /* Enqueue final Transpose Kernel */
    status = clEnqueueNDRangeKernel(
                 commandQueue,
                 kernelTranspose,
                 2,
                 NULL,
                 globalThreadsT,
                 localThreadsT,
                 0,
                 NULL,
                 &events[1]);
    if(!sampleCommon->checkVal(
                        status,
                        CL_SUCCESS, 
                        "clEnqueueNDRangeKernel failed."))

    /* Wait for transpose kernel to finish execution */
    status = clWaitForEvents(1, &events[1]);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clWaitForEvents failed."))
        return SDK_FAILURE;


    /* finish all previously enqueued commands */
    clFinish(commandQueue);

    /* compute time taken to execute the kernel */
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
                     events[1],
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


    status = clReleaseEvent(events[0]);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clReleaseEvent failed."))
        return SDK_FAILURE;

    /* Enqueue read output buffer to outputImageData */
    status = clEnqueueReadBuffer(commandQueue,
                                 outputImageBuffer,
                                 1,
                                 0,
                                 width * height * sizeof(cl_uchar4),
                                 outputImageData, 
                                 0, 0, 0);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clEnqueueReadBuffer failed."))
        return SDK_FAILURE;

    /* wait for read command to finish */
    status = clFinish(commandQueue);
    if(!sampleCommon->checkVal(
            status,
            CL_SUCCESS,
            "clFinish failed."))
        return SDK_FAILURE;

    return SDK_SUCCESS;
}



int 
RecursiveGaussian::initialize()
{
    // Call base class Initialize to get default configuration
    if(!this->SDKSample::initialize())
        return SDK_FAILURE;

    return SDK_SUCCESS;
}

int 
RecursiveGaussian::setup()
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
RecursiveGaussian::run()
{
    /* Set kernel arguments and run kernel */
    if(runCLKernels()!=SDK_SUCCESS)
        return SDK_FAILURE;

    /* write the output image to bitmap file */
	if(writeOutputImage(OUTPUT_IMAGE)!=SDK_SUCCESS)
        return SDK_FAILURE;

    return SDK_SUCCESS;
}

int 
RecursiveGaussian::cleanup()
{
    /* Releases OpenCL resources (Context, Memory etc.) */
    cl_int status;

    status = clReleaseKernel(kernelRecursiveGaussian);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseKernel(kernelRecursiveGaussian) failed."))
        return SDK_FAILURE;

    status = clReleaseKernel(kernelTranspose);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseKernel(kernelTranspose) failed."))
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
        "clReleaseMemObject(inputImageBuffer) failed."))
        return SDK_FAILURE;

    status = clReleaseMemObject(outputImageBuffer);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseMemObject(outputImageBuffer) failed."))
        return SDK_FAILURE;

    status = clReleaseMemObject(tempImageBuffer);
    if(!sampleCommon->checkVal(
        status,
        CL_SUCCESS,
        "clReleaseMemObject(tempImageBuffer) failed."))
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

    if(maxWorkItemSizes)
        free(maxWorkItemSizes);

    return SDK_SUCCESS;
}

void
RecursiveGaussian::recursiveGaussianCPU(cl_uchar4* input, cl_uchar4* output,
                                        const int width, const int height,
                                        const float a0, const float a1, 
				                        const float a2, const float a3, 
				                        const float b1, const float b2, 
				                        const float coefp, const float coefn)
{

    /* outer loop over all columns within image */
    for (int X = 0; X < width; X++)
    {
        // start forward filter pass
        float xp[4] = {0.0f, 0.0f, 0.0f, 0.0f};  // previous input
        float yp[4] = {0.0f, 0.0f, 0.0f, 0.0f};  // previous output
        float yb[4] = {0.0f, 0.0f, 0.0f, 0.0f};  // previous output by 2

        float xc[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        float yc[4] = {0.0f, 0.0f, 0.0f, 0.0f}; 

        for (int Y = 0; Y < height; Y++) 
        {
            /* output position to write */
            int pos = Y * width + X;

            /* convert input element to float4 */
            xc[0] = input[pos].u8[0];
            xc[1] = input[pos].u8[1];
            xc[2] = input[pos].u8[2];
            xc[3] = input[pos].u8[3];

            yc[0] = (a0 * xc[0]) + (a1 * xp[0]) - (b1 * yp[0]) - (b2 * yb[0]);
            yc[1] = (a0 * xc[1]) + (a1 * xp[1]) - (b1 * yp[1]) - (b2 * yb[1]);
            yc[2] = (a0 * xc[2]) + (a1 * xp[2]) - (b1 * yp[2]) - (b2 * yb[2]);
            yc[3] = (a0 * xc[3]) + (a1 * xp[3]) - (b1 * yp[3]) - (b2 * yb[3]);

            /* convert float4 element to output */
            output[pos].u8[0] = (cl_uchar)yc[0];
            output[pos].u8[1] = (cl_uchar)yc[1];
            output[pos].u8[2] = (cl_uchar)yc[2];
            output[pos].u8[3] = (cl_uchar)yc[3];

            for (int i = 0; i < 4; i++)
            {
                xp[i] = xc[i]; 
                yb[i] = yp[i]; 
                yp[i] = yc[i]; 
            }
        }
        
        // start reverse filter pass: ensures response is symmetrical
        float xn[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        float xa[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        float yn[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        float ya[4] = {0.0f, 0.0f, 0.0f, 0.0f};


        float fTemp[4] = {0.0f, 0.0f, 0.0f, 0.0f};

        for (int Y = height - 1; Y > -1; Y--) 
        {
            int pos = Y * width + X;

            /* convert uchar4 to float4 */
            xc[0] = input[pos].u8[0];
            xc[1] = input[pos].u8[1];
            xc[2] = input[pos].u8[2];
            xc[3] = input[pos].u8[3];

            yc[0] = (a2 * xn[0]) + (a3 * xa[0]) - (b1 * yn[0]) - (b2 * ya[0]);
            yc[1] = (a2 * xn[1]) + (a3 * xa[1]) - (b1 * yn[1]) - (b2 * ya[1]);
            yc[2] = (a2 * xn[2]) + (a3 * xa[2]) - (b1 * yn[2]) - (b2 * ya[2]);
            yc[3] = (a2 * xn[3]) + (a3 * xa[3]) - (b1 * yn[3]) - (b2 * ya[3]);

            for (int i = 0; i< 4; i++)
            {
                xa[i] = xn[i]; 
                xn[i] = xc[i]; 
                ya[i] = yn[i]; 
                yn[i] = yc[i];
            }

            /* convert uhcar4 to float4 */
            fTemp[0] = output[pos].u8[0];
            fTemp[1] = output[pos].u8[1];
            fTemp[2] = output[pos].u8[2];
            fTemp[3] = output[pos].u8[3];

            fTemp[0] += yc[0];
            fTemp[1] += yc[1];
            fTemp[2] += yc[2];
            fTemp[3] += yc[3];

            /* convert float4 to uchar4 */
            output[pos].u8[0] = (cl_uchar)fTemp[0];
            output[pos].u8[1] = (cl_uchar)fTemp[1];
            output[pos].u8[2] = (cl_uchar)fTemp[2];
            output[pos].u8[3] = (cl_uchar)fTemp[3];
        }
    }

}

void 
RecursiveGaussian::transposeCPU(cl_uchar4* input, cl_uchar4* output,
                                const int width, const int height)
{
    /* transpose matrix */
    for(int Y = 0; Y < height; Y++) 
    {
        for(int X = 0; X < width; X++) 
        {
            output[Y + X*height] = input[X + Y*width];
        }
    }  
}

void 
RecursiveGaussian::recursiveGaussianCPUReference()
{

    /* Create a temp uchar4 array */
    cl_uchar4* temp = (cl_uchar4*)malloc(width * height * sizeof(cl_uchar4));
    if(temp == NULL)
    {
        sampleCommon->error("Failed to allocate host memory! (temp)");
        return;
    }

    std::string inputImageName = "RecursiveGaussian_Input.bmp";

    /* Load the image again */
    inputBitmap.load(inputImageName.c_str());

    /* if image is loaded */
    

    pixelData = inputBitmap.getPixels();

    /* Copy pixel data into inputImageData */
    memcpy(inputImageData, pixelData, width * height * sizeof(cl_uchar4));

    /* Call recursive Gaussian CPU */
    recursiveGaussianCPU(inputImageData, temp, width, height, 
        oclGP.a0, oclGP.a1, oclGP.a2, oclGP.a3,
        oclGP.b1, oclGP.b2, oclGP.coefp, oclGP.coefn);
  
    /* Transpose the temp buffer */
    transposeCPU(temp, verificationOutput, width, height);

    /* again Call recursive Gaussian CPU */
    recursiveGaussianCPU(verificationOutput, temp, height, width, 
        oclGP.a0, oclGP.a1, oclGP.a2, oclGP.a3,
        oclGP.b1, oclGP.b2, oclGP.coefp, oclGP.coefn);

    /* Do a final Transpose */
    transposeCPU(temp, verificationOutput, height, width);  

    if(temp)
        free(temp);

}

/* convert uchar4 data to uint */
unsigned int rgbaUchar4ToUint(const cl_uchar4 rgba)
{
    unsigned int uiPackedRGBA = 0U;
    uiPackedRGBA |= 0x000000FF & (unsigned int)rgba.u8[0];
    uiPackedRGBA |= 0x0000FF00 & (((unsigned int)rgba.u8[1]) << 8);
    uiPackedRGBA |= 0x00FF0000 & (((unsigned int)rgba.u8[2]) << 16);
    uiPackedRGBA |= 0xFF000000 & (((unsigned int)rgba.u8[3]) << 24);
    return uiPackedRGBA;
}


int 
RecursiveGaussian::verifyResults()
{

    if(verify)
    {     
        recursiveGaussianCPUReference();

        float *outputDevice = new float[width * height * 4];
        if(outputDevice == NULL)
        {
            sampleCommon->error("Failed to allocate host memory! (outputDevice)");
            return SDK_FAILURE;
        }
        float *outputReference = new float[width * height * 4];
        if(outputReference == NULL)
        {
            sampleCommon->error("Failed to allocate host memory! (outputReference)");
            return SDK_FAILURE;
        }

        int m = 0;

        /* copy uchar4 data to float array */
        for(int i=0; i < (int)(width * height); i++)
        {
            outputDevice[4 * i + 0] = outputImageData[i].u8[0];
            outputDevice[4 * i + 1] = outputImageData[i].u8[1];
            outputDevice[4 * i + 2] = outputImageData[i].u8[2];
            outputDevice[4 * i + 3] = outputImageData[i].u8[3];

            outputReference[4 * i + 0] = verificationOutput[i].u8[0];
            outputReference[4 * i + 1] = verificationOutput[i].u8[1];
            outputReference[4 * i + 2] = verificationOutput[i].u8[2];
            outputReference[4 * i + 3] = verificationOutput[i].u8[3];
        }
     

        /* compare the results and see if they match */
        if(sampleCommon->compare(outputReference, outputDevice, width*height, (float)0.0001))
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
RecursiveGaussian::printStats()
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
    RecursiveGaussian clRecursiveGaussian("OpenCL RecursiveGaussian");
    
    if(clRecursiveGaussian.initialize()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(!clRecursiveGaussian.parseCommandLine(argc, argv))
        return SDK_FAILURE;
    if(clRecursiveGaussian.setup()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clRecursiveGaussian.run()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clRecursiveGaussian.verifyResults()!=SDK_SUCCESS)
        return SDK_FAILURE;
    if(clRecursiveGaussian.cleanup()!=SDK_SUCCESS)
        return SDK_FAILURE;
    clRecursiveGaussian.printStats();

    return SDK_SUCCESS;
}



