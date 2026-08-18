# Atari Centipede Game

## Overview
A C++ recreation of the classic Centipede-style arcade game built with SFML. The project implements a real-time game loop, player movement and shooting, enemy movement, collision detection, scoring, lives, sprites, and game-over behavior.

## Technologies
- C++
- SFML Graphics
- SFML Audio
- CMake
- Object-oriented programming
- Sprite-based 2D rendering

## Key Features
- Keyboard-controlled player movement
- Projectile / laser system
- Multi-segment centipede enemy
- Spider enemy behavior
- Mushroom obstacles with damage states
- Collision detection
- Score tracking
- Player lives and game-over state
- Sprite, font, and background assets
- Delta-time-based movement updates

## Controls
- **Arrow Keys** — Move
- **Space** — Fire
- **Enter** — Start
- **Escape** — Exit

## Project Structure
```text
Atari Centipede Game/
├── code/
│   ├── ECE_Centipede.cpp
│   ├── ECE_Centipede.h
│   ├── ECE_LaserBlast.cpp
│   ├── ECE_LaserBlast.h
│   └── main.cpp
├── fonts/
├── graphics/
└── CMakeLists.txt
```

## Build Requirements
- C++ compiler
- CMake
- SFML

A typical CMake build:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Engineering Concepts Demonstrated
- Object-oriented C++
- Real-time application loops
- Event and keyboard input handling
- Collision detection
- Dynamic containers
- Resource management
- Sprite state
- Game timing
- CMake configuration

## What I Learned
This project strengthened my understanding of real-time software behavior, including continuous input, object updates, collision detection, dynamic game objects, scoring, and rendering.
