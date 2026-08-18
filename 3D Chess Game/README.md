# 3D Chess Game

## Overview
A 3D chess application written in C++ that combines real-time OpenGL rendering with a chess engine for move generation. The program renders a chess board and pieces, accepts player moves, communicates with the Komodo chess engine through UCI commands, and applies the engine's response to the 3D scene.

## Technologies
- C++
- OpenGL
- GLFW
- GLEW
- GLM
- GLSL vertex and fragment shaders
- Multithreading and mutex synchronization
- Komodo chess engine / UCI protocol
- OBJ models and texture assets

## Key Features
- Interactive 3D chess board and piece rendering
- Komodo chess-engine integration
- UCI-based command and response handling
- Player-versus-engine gameplay
- Camera and lighting controls
- Custom vertex and fragment shaders
- 3D model and texture loading
- Thread synchronization for game, camera, and input state

## Project Structure
```text
3D Chess Game/
├── Lab3/                         # Chess piece and board assets
├── StandardShading.fragmentshader
├── StandardShading.vertexshader
├── chessCommon.h
├── chessComponent.cpp
├── chessComponent.h
├── chess_3D_view.cpp             # Main application and game loop
└── helperFile.cpp                # Chess-engine communication helpers
```

## How It Works
The application initializes an OpenGL 3.3 rendering context using GLFW and GLEW, loads the chess board and piece models, and manages the game state in C++. Player moves are sent to the Komodo engine using UCI commands. The engine calculates a response move, which is parsed and applied to the rendered board.

## Requirements
- C++ compiler with C++11 or newer support
- OpenGL 3.3+
- GLFW
- GLEW
- GLM
- Komodo chess engine
- Included model and texture assets

> **Note:** The Komodo executable/path and graphics-library paths may need to be configured for your local environment.

## Engineering Concepts Demonstrated
- Object-oriented C++
- Graphics and shader programming
- External-process communication
- Protocol parsing
- Multithreading and synchronization
- 3D transformations and camera control
- Game-state management

## What I Learned
This project provided experience integrating a graphics pipeline, game logic, user input, multithreaded state management, and an external chess engine into one application.
