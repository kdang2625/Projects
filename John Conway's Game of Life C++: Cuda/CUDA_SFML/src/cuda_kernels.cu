/*
Author: Kenny Dang
Class: ECE4122
Last Date Modified: 11/8/2024

Description: Functions used in main

*/
#include "cuda_kernels.cuh"
#include <cstdlib>
#include <ctime>
#include <iostream>

// Kernel to update the grid
__global__ void updateGridKernel(bool* currentGrid, bool* nextGrid, int gridWidth, int gridHeight) 
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < gridWidth && y < gridHeight) 
    {
        int count = 0;
        for (int i = -1; i <= 1; ++i) 
        {
            for (int j = -1; j <= 1; ++j) 
            {
                if (i == 0 && j == 0) 
                continue;
                int nx = (x + i + gridWidth) % gridWidth;
                int ny = (y + j + gridHeight) % gridHeight;
                count += currentGrid[ny * gridWidth + nx];
            }
        }
        int i = y * gridWidth + x;
        if (currentGrid[i]) 
        {
            nextGrid[i] = (count == 2 || count == 3);
        }
        else 
        {
            nextGrid[i] = (count == 3);
        }
    }
}

// Initialize the grid with random values
void seedRandomGrid(bool* grid, int gridWidth, int gridHeight) 
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < gridWidth * gridHeight; ++i)
    {
        grid[i] = (std::rand() % 2 == 0);
    }
}

// Run the Game of Life using CUDA
void runGameOfLife(bool* d_currentGrid, bool* d_nextGrid, int gridWidth, int gridHeight, dim3 threadsPerBlock, dim3 numBlocks) 
{
    updateGridKernel << <numBlocks, threadsPerBlock >> > (d_currentGrid, d_nextGrid, gridWidth, gridHeight);
    cudaDeviceSynchronize();
}
