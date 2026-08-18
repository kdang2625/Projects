/*
Author: Kenny Dang
Class: ECE4122
Last Date Modified: 11/8/2024

Description: Main file to run the executable ( ./Lab4  ARGS )

-n <number of threads>
-c <size of each cell in pixels>
-x <window width in pixels>
-y <window height in pixels>
-t <memory mode>
	(NORMAL, PINNED, and MANAGED)
*/



#include <SFML/Graphics.hpp>
#include "cuda_kernels.cuh"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cuda_runtime.h>
#include <chrono>


int main(int argc, char* argv[]) {
    // Default grid dimensions
    int windowWidth = 800;
    int windowHeight = 600;
    int pixelSize = 5;
    int threadsPerBlock = 32;
    std::string memType = "NORMAL";

    // Parse command line arguments, taken from lab 2
    for (int i = 1; i < argc; ++i) 
    {
        std::string arg = argv[i];
        // Change width
        if (arg == "-x" && i + 1 < argc) 
        {
            windowWidth = std::stoi(argv[++i]);
        }
        // Change height
        if (arg == "-y" && i + 1 < argc) 
        {
            windowHeight = std::stoi(argv[++i]);
        }
        // Changing pixel size
        if (arg == "-c" && i + 1 < argc) 
        {
            pixelSize = std::stoi(argv[++i]);
        }
        // Changing number of threads
        if (arg == "-n" && i + 1 < argc) 
        {
            threadsPerBlock = std::stoi(argv[++i]);
        }
        // Changing memory type
        if (arg == "-t" && i + 1 < argc) 
        {
            memType = argv[++i];
        }
    }

    int widthOfGrid = windowWidth / pixelSize;
    int heightOfGrid = windowHeight / pixelSize;

    // Allocate memory for the grids based on memory type
    bool* currGrid, * nextGrid;

    if(memType == "NORMAL")
    {
        cudaMalloc(&currGrid, widthOfGrid * heightOfGrid * sizeof(bool));
        cudaMalloc(&nextGrid, widthOfGrid * heightOfGrid * sizeof(bool));
    }
    if (memType == "PINNED") 
    {
        cudaMallocHost(&currGrid, widthOfGrid * heightOfGrid * sizeof(bool));
        cudaMallocHost(&nextGrid, widthOfGrid * heightOfGrid * sizeof(bool));
    }
    if (memType == "MANAGED") 
    {
        cudaMallocManaged(&currGrid, widthOfGrid * heightOfGrid * sizeof(bool));
        cudaMallocManaged(&nextGrid, widthOfGrid * heightOfGrid * sizeof(bool));
    }
    

    // Random spots for the pixels
    bool* initialGrid = new bool[widthOfGrid * heightOfGrid];
    seedRandomGrid(initialGrid, widthOfGrid, heightOfGrid);
    cudaMemcpy(currGrid, initialGrid, widthOfGrid * heightOfGrid * sizeof(bool), cudaMemcpyHostToDevice);
    delete[] initialGrid;

    // Allocate memory on the host for visualization
    bool* currentGridHost = new bool[widthOfGrid * heightOfGrid];

    // Define the CUDA thread and block dimensions
    dim3 threadsPerBlockDim(32, 32);
    dim3 numBlocks((widthOfGrid + threadsPerBlockDim.x - 1) / threadsPerBlockDim.x,
        (heightOfGrid + threadsPerBlockDim.y - 1) / threadsPerBlockDim.y);

    // Set up the SFML window
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "CUDA-based Game of Life");
    window.setFramerateLimit(120);

    // Main loop
    unsigned long generationCount = 0;
    double timeFor100Gens = 0.0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                window.close();
            }
        }

        // Start timing calculations
        cudaEvent_t startTime;
        cudaEvent_t endTime;
        cudaEventCreate(&startTime);
        cudaEventCreate(&endTime);
        cudaEventRecord(startTime);
        // Run CUDA kernel to update the grids
        runGameOfLife(currGrid, nextGrid, widthOfGrid, heightOfGrid, threadsPerBlockDim, numBlocks);

        // Swap the grids
        std::swap(currGrid, nextGrid);
        cudaMemcpy(currentGridHost, currGrid, widthOfGrid * heightOfGrid * sizeof(bool), cudaMemcpyDeviceToHost);

        // Stop timing calculations
        cudaEventRecord(endTime);
        cudaEventSynchronize(endTime);

        float generationTime = 0.0f;
        cudaEventElapsedTime(&generationTime, startTime, endTime);
        timeFor100Gens += generationTime;
        generationCount++;


        // Draw the current grid
        window.clear();
        for (int y = 0; y < heightOfGrid; ++y) 
        {
            for (int x = 0; x < widthOfGrid; ++x) 
            {
                if (currentGridHost[y * widthOfGrid + x]) 
                {
                    sf::RectangleShape cell(sf::Vector2f(pixelSize, pixelSize));
                    cell.setPosition(x * pixelSize, y * pixelSize);
                    cell.setFillColor(sf::Color::White);
                    window.draw(cell);
                }
            }
        }
        window.display();

        // Display timing information every 100 frames
        if (generationCount == 100) 
        {
            std::cout << "100 generations took " << timeFor100Gens << " microseconds." << std::endl;
            generationCount = 0;
            timeFor100Gens = 0.0;
        }
    }

    // Free the allocated host memory
    delete[] currentGridHost;

    // Free GPU memory
    if (memType == "PINNED") 
    {
        cudaFreeHost(currGrid);
        cudaFreeHost(nextGrid);
    } else 
    {
        cudaFree(currGrid);
        cudaFree(nextGrid);
    }

    return 0;
}
