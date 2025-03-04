---
title:    HIP and GPU kernels
subtitle: GPU programming with HIP
author:   CSC Training
date:     2025-03
lang:     en
---

# HIP

::: incremental
- HIP = Heterogeneous-computing Interface for Portability
- AMD effort to offer a common programming interface that works on both
      CUDA and ROCm devices
- HIP is a C++ runtime API **and** a kernel programming language
    - standard C++ syntax, uses nvcc/hcc compiler in the background
    - almost a one-on-one clone of CUDA from the user perspective
    - allows one to write portable GPU codes
- AMD offers also a wide set of optimised libraries and tools
:::

# AMD GPU terminology

<small>

::: incremental
- Compute Unit
    - a parallel vector processor in a GPU
- kernel
    - parallel code executed on the GPU
- thread
    - individual worker of a wavefront
- wavefront (cf. CUDA warp)
    - collection of threads that execute in lockstep and execute the same
      instructions
    - each wavefront has fixed number of threads (AMD: 64, NVIDIA 32)
    - the number of threads, and thus implicitly the number of wavefronts, per workgroup is chosen at kernel launch
- workgroup (cf. CUDA thread block)
    - group of threads that are on the GPU at the same time and
      are part of the same compute unit (CU)
    - can synchronise together and communicate through memory in the CU
</small>
:::

# HIP programming model

::: incremental
- GPU accelerator is often called a *device* and CPU a *host*
- parallel code is
    - launched by the host using the HIP API
    - written using the kernel language
        - from the point of view of a single thread
        - each thread has a unique ID
    - executed on a device by many threads
:::

# GPU programming considerations

::: incremental
- parallel nature of GPUs requires many similar tasks that can be executed simultaneously
    - one usage is to replace iterations of loop with a GPU kernel call
- need to adapt CPU code to run on the GPU
    - algorithmic changes to fit the parallel execution model
    - share data among hundreds of cooperating threads
    - manage data transfers between CPU and GPU memories
      carefully (a common bottleneck)
:::

# HIP API

Code on the CPU to control the larger context and the flow of execution

::: incremental
- device init and management: `hipSetDevice`
- memory management: `hipMalloc`
- execution control: `kernel<<<blocks, threads>>>`
- synchronisation: device, stream, events: `hipDeviceSynchronize`
- error handling, context handling, ... : `hipGetErrorString`
:::

# API example: Hello world

```cpp
#include <hip/hip_runtime.h>
#include <stdio.h>

int main(void)
{
    int count, device;

    hipGetDeviceCount(&count);
    hipGetDevice(&device);

    printf("Hello! I'm GPU %d out of %d GPUs in total.\n", device, count);

    return 0;
}
```

# HIP kernels

Code on the GPU from the point of view of a single thread

<small>

:::::: {.column width=45%}
::: incremental
- kernel is a function executed by the GPU
- kernel must be declared with the `__global__` attribute and the return type must be `void`
- any function called from a kernel must be declared with `__device__` attribute
- all pointers passed to a kernel should point to memory accessible from
  the device
- unique thread and block IDs can be used to distribute work
:::
::::::
:::::: {.column width=45%}
- attributes: `__device__`, `__global__`, `__shared__`, ...
- built-in variables: `threadIdx.x`, `blockIdx.y`, ...
- vector types: `int3`, `float2`, `dim3`, ...
- math functions: `sqrt`, `powf`, `sinh`, ...
- atomic functions: `atomicAdd`, `atomicMin`, ...
- intrinsic functions: `__syncthreads`, `__threadfence`, ...
::::::

</small>

# Kernel example: axpy

```cpp
__global__ void axpy_(int n, double a, double *x, double *y)
{
    int tid = threadIdx.x + blockIdx.x * blockDim.x;

    if (tid < n) {
        y[tid] += a * x[tid];
    }
}
```

::: incremental
- global ID `tid` calculated based on the thread and block IDs
    - only threads with `tid` smaller than `n` calculate
    - works only if number of threads ≥ `n`
:::


# Kernel example: axpy (revisited)

```cpp
__global__ void axpy_(int n, double a, double *x, double *y)
{
    int tid = threadIdx.x + blockIdx.x * blockDim.x;
    int stride = gridDim.x * blockDim.x;

    for (; tid < n; tid += stride) {
        y[tid] += a * x[tid];
    }
}
```

- handles any vector size, but grid size should still be chosen with some care

# Grid: thread hierarchy

<small>

:::::: {.column width=40%}
::: incremental
- kernels are executed on a 3D *grid* of threads
- threads are partitioned into equal-sized *blocks*
- code is executed by the threads, the grid is a way to organise the
  work
- dimension of the grid are set at kernel launch
- built-in variables to be used within a kernel: `threadIdx`, `blockIDx`, `blockDim`, `gridDim`
:::
::::::
:::::: {.column width=55%}
![](img/software_hardware_mapping.png){.center width=70%}
::::::

</small>

# Launching kernels

::: incremental
- kernels are launched with one of the two following options:
  - CUDA syntax (recommended, because it works on CUDA and HIP both):
  ```cpp
  somekernel<<<blocks, threads, shmem, stream>>>(args)
  ```
  
  - HIP syntax:
  ```cpp
  hipLaunchKernelGGL(somekernel, blocks, threads, shmem, stream, args)
  ```

- grid dimensions are obligatory
    - must have an integer type or vector type of `dim3`
- `shmem`, and `stream` are optional arguments for CUDA syntax, and can be `0` for the HIP syntax
- kernel execution is asynchronous with the host
:::

# Simple memory management

::: incremental
- GPU has it's own memory area
    - allocate device usable memory with `hipMalloc` (cf. `cudaMalloc` and `std::malloc`)
    - pass the pointer to the kernel
- copy data to/from device: `hipMemcpy` (cf. `cudaMemcpy`, `std::memcpy`)
:::

```cpp
double *x_
hipMalloc((void **) &x_, sizeof(double) * n);
hipMemcpy(x_, x, sizeof(double) * n, hipMemcpyHostToDevice);
hipMemcpy(x, x_, sizeof(double) * n, hipMemcpyDeviceToHost);
```

# Error checking

<small>

::: incremental
- always use HIP error checking with larger codebases!
  - it has low overhead, and can save a lot of debugging time!
- for teaching purposes many exercises of this course do not have error checking
:::

```cpp
#define HIP_ERR(err) (hip_error(err, __FILE__, __LINE__))
inline static void hip_error(hipError_t err, const char *file, int line) {
  if (err != hipSuccess) {
    printf("\n\n%s in %s at line %d\n", hipGetErrorString(err), file, line);
    exit(1);
  }
}

 // Wrap API call with the macro
inline static void* alloc(size_t bytes) {
  void* ptr;
  HIP_ERR(hipMallocManaged(&ptr, bytes));
  return ptr;
}

```
</small>


# Example: fill (complete device code and launch)

<small>
<div class="column">
```cpp
#include <hip/hip_runtime.h>
#include <stdio.h>

__global__ void fill_(int n, double *x, double a)
{
    int tid = threadIdx.x + blockIdx.x * blockDim.x;
    int stride = gridDim.x * blockDim.x;

    for (; tid < n; tid += stride) {
        x[tid] = a;
    }
}
```
</div>

<div class="column">
```cpp
int main(void)
{
    const int n = 10000;
    double a = 3.4;
    double x[n];
    double *x_;

    // allocate device memory
    hipMalloc(&x_, sizeof(double) * n);

    // launch kernel
    dim3 blocks(32);
    dim3 threads(256);
    fill_<<<blocks, threads>>>(n, x_, a);

    // copy data to the host and print
    hipMemcpy(x, x_, sizeof(double) * n, hipMemcpyDeviceToHost);
    printf("%f %f %f %f ... %f %f\n",
            x[0], x[1], x[2], x[3], x[n-2], x[n-1]);

    return 0;
}
```
</div>
</small>


# Summary

::: incremental
- HIP supports both AMD and NVIDIA GPUs
- HIP consists of an API and a kernel language
    - API controls the larger context
    - kernel language for single thread point of view GPU code
- kernels execute over a grid of (blocks of) threads
    - each block is executed in wavefronts of 64 (AMD) or 32 (NVIDIA) threads
- kernels need to be declared `__global__` and `void` and are launched with special syntax
:::
