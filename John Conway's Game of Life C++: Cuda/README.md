# CUDA-Accelerated Conway's Game of Life

## Overview
A GPU-accelerated implementation of Conway's Game of Life written in C++ and CUDA, with SFML used to visualize the simulation in real time.

The cellular-automata update runs in a CUDA kernel so cells can be processed in parallel on the GPU. The application supports configurable grid/window sizes, CUDA thread-block sizing, and multiple CUDA memory-allocation modes.

## Technologies
- C++
- NVIDIA CUDA
- SFML
- CMake
- GPU parallel programming
- CUDA memory management
- Command-line argument parsing

## Key Features
- CUDA kernel for parallel Game of Life updates
- Real-time SFML visualization
- Random initial grid generation
- Toroidal / wraparound grid boundaries
- Configurable CUDA thread count
- Configurable cell size and window dimensions
- Memory modes: `NORMAL`, `PINNED`, and `MANAGED`

## Command-Line Options
```text
-n <number>   CUDA threads per block
-c <number>   Cell size in pixels
-x <number>   Window width in pixels
-y <number>   Window height in pixels
-t <type>     Memory type: NORMAL, PINNED, or MANAGED
```

Example:
```bash
./Lab4 -n 32 -c 5 -x 800 -y 600 -t MANAGED
```

## Project Structure
```text
John Conway's Game of Life C++: Cuda/
├── CMakeLists.txt
└── CUDA_SFML/
    ├── CMakeLists.txt
    └── src/
        ├── cuda_kernels.cu
        ├── cuda_kernels.cuh
        └── main.cpp
```

## How It Works
Each CUDA thread updates one cell. The kernel counts the eight neighboring cells and applies Conway's rules. Wraparound calculations allow cells at one edge to interact with cells on the opposite edge.

## Requirements
- NVIDIA GPU with CUDA support
- NVIDIA CUDA Toolkit
- C++ compiler
- SFML
- CMake

## Building
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Engineering Concepts Demonstrated
- GPU parallelism
- CUDA kernels and thread indexing
- Device synchronization
- CUDA memory-management strategies
- C++ / CUDA interoperability
- Real-time visualization
- Performance tuning
- Command-line interface design

## What I Learned
This project provided hands-on experience moving a naturally parallel algorithm to GPU execution and comparing implementation choices such as memory type and thread configuration.
