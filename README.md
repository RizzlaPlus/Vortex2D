![Build Status](https://github.com/mmaldacker/Vortex2D/actions/workflows/build.yml/badge.svg)
[![Docs](https://readthedocs.org/projects/vortex2d/badge/?version=latest)](https://vortex2d.readthedocs.io)

# Vortex2D

A C++ real time fluid simulation engine based on the incompressible Navier-Stokes equation.
This implementation is a hybrid method using particles for advection and a grid for solving the incompressible equations.
All computing is done on the GPU with Vulkan using compute shaders.

Documentation can be found at https://vortex2d.readthedocs.io.

## Examples

<p align="middle">
  <img src="https://github.com/mmaldacker/Vortex2D/raw/master/Docs/vortex2d_example1.gif " width="350"/> 
  <img src="https://github.com/mmaldacker/Vortex2D/raw/master/Docs/vortex2d_example2.gif " width="350"/> 
</p>

<p align="middle">
  <img src="https://github.com/mmaldacker/Vortex2D/raw/master/Docs/vortex2d_example3.gif " width="350"/>
  <img src="https://github.com/mmaldacker/Vortex2D/raw/master/Docs/vortex2d_example4.gif " width="350"/> 
</p>

Various scenes can be selected in the example application with the number keys 1 till 8.

## Quick start

### Linux

Get the LunarG Vulkan SDK, and extract somewhere. Vortex2D requires the `glslangValidator` compiler which is installed with the SDK. Some env vars need to be set so cmake can find the SDK: `source setup-env.sh`.

Vortex2D can then be built with cmake as so:

```
mkdir build && cd build
cmake .. -DVORTEX2D_ENABLE_EXAMPLES=On -DVORTEX2D_ENABLE_TESTS=On
make -j 4
```

### Windows

Get the lunarG Vulkan SDK and install. That's it, you can then build Vortex2D with cmake. Start the cmake-gui, select the source and build folder and configure. Select `VORTEX2D_ENABLE_EXAMPLES` to also build the examples.

### Mac

Get the LunarG Vulkan SDK or build MoltenVK directly. Vortex2D requires the `glslangValidator` compiler which  is installed with the SDK. 

Vortex2D can then be built with cmake as so:

```
mkdir build && cd build
cmake .. -DVORTEX2D_ENABLE_EXAMPLES=On -DVORTEX2D_ENABLE_TESTS=On -DMOLTENVK_DIR=path_to_sdk/MoltenVK -DGLSL_VALIDATOR=path_to_bin/glslangValidator
make -j 4
```

## Features

 * 3rd order runge kutta interpolation
 * Bicubic interpolation
 * Multi grid preconditioned conjugate gradient linear solver
 * FLIP & PIC hybrid method
 * Strong rigid body coupling
 * Level set re-initialisation