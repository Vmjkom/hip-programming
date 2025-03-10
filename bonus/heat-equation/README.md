# Bonus: Heat equation solver with HIP

Create a parallel version of a heat equation solver using HIP.

Starting from a [serial heat equation solver](serial) (see below for details),
port the code to GPUs using HIP. Main computational routine is the time
evolution loop in the `core.cpp` file.

Alternatively, you may start from a [CUDA+MPI version](cuda) and hipify the code to jump start the work.

Note: You may need to comment out the PNG generation parts, if the system you
are using doesn't have libpng installed.

## Heat equation solver

The heat equation is a partial differential equation that describes the
variation of temperature in a given region over time

$$\frac{\partial u}{\partial t} = \alpha \nabla^2 u $$

where u(x, y, z, t) represents temperature variation over space at a given
time, and α is a thermal diffusivity constant.

We limit ourselves to two dimensions (plane) and discretize the equation onto
a grid.  The two dimensional Laplacian can be
discretized with finite differences as

```math
\nabla^2 u  = \frac{u(i-1,j)-2u(i,j)+u(i+1,j)}{(\Delta x)^2} + \frac{u(i,j-1)-2u(i,j)+u(i,j+1)}{(\Delta y)^2}

```
 Given an initial condition (u(t=0) = u0) one can follow the time dependence
 of
 the temperature field with explicit time evolution method:

$$u^{m+1}(i,j) = u^m(i,j) + \Delta t \alpha \nabla^2 u^m(i,j) $$

 Note: Algorithm is stable only when

$$ \Delta t < \frac{1}{2 \alpha} \frac{(\Delta x \Delta y)^2}{(\Delta x)^2+
 (\Delta y)^2} $$

There is a solver for the 2D equation implemented in C++ and Fortran. You can
compile the program by adjusting the Makefile as needed and typing `make`. The
solver carries out the time development of the 2D heat equation over the
number of time steps provided by the user. The default geometry is a flat
rectangle (with grid size provided by the user), but other shapes may be used
via input files. Examples on how to run the binary:

- `./heat`
  No arguments - the program will run with the default arguments: 200 x 200
  grid and 500 time steps
- `./heat ../common/bottle.dat`
  One argument - start from a temperature grid provided in the given file
  for the default number of time steps
- `./heat ../common/bottle.dat 1000`
  Two arguments - will run the program starting from a temperature grid
  provided in the given file for 1000 time steps
- `./heat 1024 2048 1000`
  Three arguments - will run the program using the grid dimensions (1024 x 2048) and the number of time steps (1000) specified by the arguments

The program will produce a `.png` image of the temperature field after every
100 iterations. You can change that from the parameter `image_interval`. You
can visualise the images using the command animate: `animate heat_*.png`, or
by using `eog heat_000.png` and using the arrow-keys to loop backward or
forward through the files.
