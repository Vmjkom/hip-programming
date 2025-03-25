#include <cstdio>
#include <hip/hip_runtime.h>
#include <iostream>

#define HIP_CHECK(expression)                  \
{                                              \
    const hipError_t status = expression;      \
    if(status != hipSuccess){                  \
        std::cerr << "HIP error "              \
                  << status << ": "            \
                  << hipGetErrorString(status) \
                  << " at " << __FILE__ << ":" \
                  << __LINE__ << std::endl;    \
    }                                          \
}

/* Blocksize divisible by the warp size */
#define BLOCKSIZE 64

/* Example struct to practise copying structs with pointers to device memory */
typedef struct
{
  float *x;
  int *idx;
  int size;
} Example;

/* GPU kernel definition */
__global__ void hipKernel(Example* const d_ex)
{
  const int thread = blockIdx.x * blockDim.x + threadIdx.x;
  if (thread < d_ex->size)
  {
    printf("x[%d]: %.2f, idx[%d]:%d/%d \n", thread, d_ex->x[thread], thread, d_ex->idx[thread], d_ex->size - 1);
  }
}

/* Run on host */
void runHost()
{
  // Allocate host struct
  Example *ex;
  ex = (Example*)malloc(sizeof(Example));
  ex->size = 10;

  // Allocate host struct members
  ex->x = (float*)malloc(ex->size * sizeof(float));
  ex->idx = (int*)malloc(ex->size * sizeof(int));

  // Initialize host struct members
  for(int i = 0; i < ex->size; i++)
  {
    ex->x[i] = (float)i;
    ex->idx[i] = i;
  }

  // Print struct values from host
  printf("\nHost:\n");
  for(int i = 0; i < ex->size; i++)
  {
    printf("x[%d]: %.2f, idx[%d]:%d/%d \n", i, ex->x[i], i, ex->idx[i], ex->size - 1);
  }

  // Free host struct
  free(ex->x);
  free(ex->idx);
  free(ex);
}

/* Run on device using Unified Memory */
void runDeviceUnifiedMem()
{
  Example *ex;
  /* #error Allocate struct using Unified Memory */
  HIP_CHECK(hipMallocManaged((void**) &ex, sizeof(Example)));

  ex->size = 10;
  /* #error Allocate struct members using Unified Memory */
  HIP_CHECK(hipMallocManaged((void**) &(ex->x), sizeof(float) * ex->size));
  HIP_CHECK(hipMallocManaged((void**) &(ex->idx), sizeof(int) * ex->size));


  // Initialize struct from host
  for(int i = 0; i < ex->size; i++)
  {
    ex->x[i] = (float)i;
    ex->idx[i] = i;
  }

  dim3 grid(1,1,1);
  dim3 block(16,16,1);
/* #error Print struct values from device by calling hipKernel() */
  hipKernel<<<grid, block, 0, 0>>>(ex);
  printf("\nDevice (UnifiedMem):\n");

  hipFree(ex->x); hipFree(ex->idx); hipFree(ex);
}

#if 0
/* Create the device struct (needed for explicit memory management) */
Example* createDeviceExample(Example *ex)
{
  #error Allocate device struct

  #error Allocate device struct members

  #error Copy arrays pointed by the struct members from host to device

  #error Copy struct members from host to device

  #error Return device struct
}
#endif

#if 0
/* Free the device struct (needed for explicit memory management) */
void freeDeviceExample(Example *d_ex)
{
  #error Copy struct members (pointers) from device to host

  #error Free device struct members

  #error Free device struct
}
#endif

#if 0
/* Run on device using Explicit memory management */
void runDeviceExplicitMem()
{
  #error Allocate host struct

  #error Allocate host struct members

  // Initialize host struct
  for(int i = 0; i < ex->size; i++)
  {
    ex->x[i] = (float)i;
    ex->idx[i] = i;
  }

  // Allocate device struct and copy values from host to device
  Example *d_ex = createDeviceExample(ex);

  #error Print struct values from device by calling hipKernel()
  printf("\nDevice (ExplicitMem):\n");

  // Free device struct
  freeDeviceExample(d_ex);

  #error Free host struct
}
#endif

/* The main function */
int main(int argc, char* argv[])
{
  runHost();
  runDeviceUnifiedMem();
  /* runDeviceExplicitMem(); */
}
