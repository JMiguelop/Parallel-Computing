/**************************************************************
 * 
 * --== Simple CUDA kernel ==--
 * author: ampereira
 * 
 *
 * Fill the rest of the code through the following steps:
 * -> allocate the device memory
 * -> copy the inputs to the device
 * -> call the kernel
 * -> copy the output to the host
 * 
 * extra points for
 * -> reversing the output array on the device
 *
 **************************************************************/
#include <stdio.h>
#include <cstdlib>
#include <iostream>

#define NUM_BLOCKS 128
#define NUM_THREADS_PER_BLOCK 256
#define SIZE NUM_BLOCKS*NUM_THREADS_PER_BLOCK

using namespace std;

void checkCUDAError (const char *msg) {
	cudaError_t err = cudaGetLastError();
	if( cudaSuccess != err) {
		cerr << "Cuda error: " << msg << ", " << cudaGetErrorString( err) << endl;
		exit(-1);
	}
}

// Fill the input parameters and kernel qualifier
__global__ void vecAdditionKernel (float *a, float *b, float *c) {
	//for(int i = 0; i < SIZE; i++) {
	//	c[i] = a[i] + b[i];
	//}
	int tid = threadIdx.x + blockIdx.x * blockDim.x;
	c[tid] = a[tid] + b[tid];
}

int main( int argc, char** argv) {
	// declare variable with size of the array in bytes
	int bytes = SIZE*sizeof(float);

	// arrays on the host
	float a[SIZE], b[SIZE], c[SIZE];

	// pointers to the device memory
	float *dev_a, *dev_b, *dev_c;

	// fills the arrays
	for (unsigned i = 0; i < SIZE; ++i) {
		a[i] = (float) rand() / RAND_MAX;
		b[i] = (float) rand() / RAND_MAX;
	}

	// allocate the memory on the device
	cudaMalloc((void**)&dev_a, bytes);
	cudaMalloc((void**)&dev_b, bytes);
	cudaMalloc((void**)&dev_c, bytes);

	checkCUDAError("mem allocation");

	

	// copy inputs to the device
	cudaMemcpy(dev_a, &a, sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy(dev_b, &b, sizeof(float), cudaMemcpyHostToDevice);	
	
	checkCUDAError("memcpy h->d");

	// launch the kernel
	dim3 dimGrid(NUM_BLOCKS);
	dim3 dimBlock(NUM_THREADS_PER_BLOCK);
	vecAdditionKernel <<< dimBlock, dimGrid >>> (dev_a, dev_b, dev_c);

	checkCUDAError("kernel invocation");

	// copy the output to the host
	cudaMemcpy(&c, dev_c, bytes, cudaMemcpyDeviceToHost);	
	
	// free the device memory
	cudaFree(dev_a);
	cudaFree(dev_b);
	cudaFree(dev_c);

	checkCUDAError("mem free");

	//Print result just for feedback
	std::cout << "finished adding matrix\n";

	return 0;
}
