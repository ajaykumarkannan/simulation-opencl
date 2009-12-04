/*
 * For a description of the algorithm and the terms used, please see the
 * documentation for this sample.
 *
 * Each thread calculates a pixel component(rgba), by applying a filter 
 * on group of 8 neighbouring pixels in both x and y directions. 
 * Both filters are summed (vector sum) to form the final result.
 */

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable  

__kernel void sobel_filter(__constant uchar* inputImage, __global uchar* outputImage)
{
	uint x = get_global_id(0);
      uint y = get_global_id(1);

	uint width = get_global_size(0);
	uint height = get_global_size(1);

	int Gx,Gy;

	/* Read each texel component and calculate the filtered value using neighbouring texel components */

	if( x >= 4 && x < (width-4) && y >= 1 && y < height - 1)
	{
		Gx =    inputImage[(x - 4) + width * (y - 1)]  + 2*inputImage[x + width * (y - 1)]  + inputImage[(x + 4) + width * (y - 1)]  
		        //+ inputImage[(x - 4) + width * (y - 0)]  + inputImage[x + width * (y - 0)]  + inputImage[(x + 4) + width * (y - 0)]  
		        - inputImage[(x - 4) + width * (y + 1)]  - 2*inputImage[x + width * (y + 1)]  - inputImage[(x + 4) + width * (y + 1)];

		Gy =    inputImage[(x - 4) + width * (y - 1)]  + 0*inputImage[x + width * (y - 1)]  - inputImage[(x + 4) + width * (y - 1)]  
		        + 2*inputImage[(x - 4) + width * (y - 0)]  + 0*inputImage[x + width * (y - 0)]  - 2*inputImage[(x + 4) + width * (y - 0)]  
		        + inputImage[(x - 4) + width * (y + 1)]  + 0*inputImage[x + width * (y + 1)]  - inputImage[(x + 4) + width * (y + 1)];

		/* taking root of sums of squares of Gx and Gy */
		outputImage[x + y * width] = hypot(Gx,Gy)/2;

	}
}

	

	 






	

	




	

	

	
	