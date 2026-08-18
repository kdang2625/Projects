/*
Author: Kenny Dang
Class: ECE4122
Last Date Modified: 12/2/2024
Description:
3-D Chess game that uses the komodo engine to calculate moves that a player sends it
*/
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Including   /////////////////////////////////////////////////
/////////////////////////////////// Headers    /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow* window;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include "chessComponent.h"
#include "chessCommon.h"
#include <iostream>
#include <string>
#include <set>
#include <thread>
#include <mutex>
#include <sstream>
#include <memory>

//including a helper file
#include "helperFile.cpp"


std::mutex mutexGame;
std::mutex mutexCam;
std::mutex mutexLight;

//Class that interfaces to the komodo engine
class ECE_ChessEngine {
public:
    std::string moveResponse = "position startpos moves";
    std::string engineMove;
    bool InitializeEngine() // starting engine here
    {
        StartEngine();
        SendToEngine("uci");
        std::cout << "Engine Response: " << ReadFromEngine() << std::endl;
        SendToEngine("isready");
        std::cout << "Engine Response: " << ReadFromEngine() << std::endl;
        return true;
    }

    // send move to Komodo, get the best move back (only thinking 3 steps ahead)
    bool sendMove(const std::string& strMove) // ON THE PDF
    {
        moveResponse = moveResponse + " " + strMove;
        std::cout << "Move sent to engine: " << moveResponse << "\n";
        SendToEngine(moveResponse);
        SendToEngine("go depth 6");
        return true;
    }
    //Function to get a move from komodo
    bool getResponseMove(std::string& strMove) // ON THE PDF
    {
        std::string response;
        std::string substr;
        while ((response = ReadFromEngine()).find("bestmove") == std::string::npos) {
            
        }
        size_t kResponse = response.find("bestmove");
        if (kResponse != std::string::npos) { 
            substr = response.substr(kResponse + 9, 4); 
        }
        else {
            std::cout << "Could not get best move from response!" << std::endl;
            return false;
        }
        std::cout << "Komodos best move: " << response << std::endl;
        moveResponse = moveResponse + " " + substr;
        engineMove = substr;
        std::cout << "ENGINE RESPONSE: " << substr << "\n";
        return true;
    }

    // checks if command line format is valid
    bool isValidFormat(const std::string& move) {
        if (move.length() != 4) {
            return false;
        }        
        char c0 = move[0]; // Check if the first two characters are valid
        char c1 = move[1];
        if (c0 < 'a' || c0 > 'h' || c1 < '1' || c1 > '8') {
            return false;
        }
        char c2 = move[2]; // Check if the last two characters are valid
        char c3 = move[3];
        if (c2 < 'a' || c2 > 'h' || c3 < '1' || c3 > '8') {
            return false;
        }
        return true;
    }

};

// Sets up the chess board
void setupChessBoard(tModelMap& cTModelMap);


void parseUCI(const std::string& uciMove, float& posX1, float& posY1, float& posX2, float& posY2) {

    float x1 = (uciMove[0] - 'a') - 3.5f;
    float y1 = (uciMove[1] - '1') - 3.5f;
    float x2 = (uciMove[2] - 'a') - 3.5f;
    float y2 = (uciMove[3] - '1') - 3.5f;

    // Calculate the positions
    posX1 = CHESS_BOX_SIZE * x1;
    posY1 = CHESS_BOX_SIZE * y1;
    posX2 = CHESS_BOX_SIZE * x2;
    posY2 = CHESS_BOX_SIZE * y2;
}

std::vector<float> isValidCommand(const std::string& input) {

    std::vector <float> wrongVal = { -1,0,0,0 };
    float functionIndex = -1;
    std::vector<float> returnVal = { functionIndex, 0, 0, 0 };
    const std::set<std::string> validCommands = { "camera", "move", "light", "quit" };

    // light/camera/move
    size_t delimiter = input.find(' ');
    std::string firstWord = (delimiter == std::string::npos) ? input : input.substr(0, delimiter);

    // Check if the first word is valid.
    if (validCommands.find(firstWord) == validCommands.end()) {
        std::cout << "Invalid Command or Move!! Valid commands are camera, move, light, or quit\n";
        return wrongVal;
    }

    if (firstWord == "camera" || firstWord == "light") {
        if (firstWord == "camera")
            functionIndex = 1;
        if (firstWord == "light")
            functionIndex = 2;
        //Edge case for if there are no arguments provided
        if (delimiter == std::string::npos) {
            printf("Invalid command or move!!\nMissing arguments\n");
            return wrongVal;
        }

        std::string arguments = input.substr(delimiter + 1);

        // Parse the arguments
        std::istringstream stream(arguments);
        float value1, value2, value3;

        //Split the stream of arguments and assign them to value1, value2, value3
        if (!(stream >> value1 >> value2 >> value3) || !stream.eof()) {
            printf("Invalid command or move!!\n Light or Camera take 3 arguments\n");
            return wrongVal;
        }

        // Validate the ranges
        if (value1 < 10 || value1 > 80) {
            printf("Invalid Command or Move!\nTheta must be in range of 10 to 80\n");
            return wrongVal;
        }
        if (value2 < 0 || value2 > 360) {
            printf("Invalid command or move!!\nPhi must be in range of 0 to 360\n");
            return wrongVal;
        }
        if (value3 <= 0) {
            printf("Invalid command or move!!\nR must be positive\n");
            return wrongVal;
        }
        if (functionIndex == 0)
            printf("Invalid command or move!!!\nERROR!!!!\n");

        returnVal = { functionIndex, value1, value2, value3 };
        return returnVal;
    }
    
    if (firstWord == "move")
    {
        return { 3, 0, 0, 0 };
    }
    return returnVal;
}

int main(void)
{
    
    ECE_ChessEngine engine;
    engine.InitializeEngine(); //Start Komodo    

    // Initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    //GLOBALS
    bool gameOver = false;
    int userInput = 0;
    float theta = 0;
    float phi = 0;
    float R(0);
    float thetaLight = 0;
    float phiLight = 0;
    float RLight = 15;
    float lightX = 0;
    float lightY = 0;
    float lightZ = 15;
    std::string moveCommand(" ");
    std::string opponentMove(" ");
    std::mutex moveCommandMutex;
    std::mutex opponentMoveMutex;


    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "4122 Final Project", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

    // Get a handle for our "lightToggleSwitch" uniform
    GLuint LightSwitchID = glGetUniformLocation(programID, "lightSwitch");


    std::vector<glm::mat4> matrixModels;
    std::vector<size_t> components;
    std::vector<tPosition> positions;
    // Create a vector of chess components class
    // Each component is fully self sufficient
    std::vector<chessComponent> gchessComponents;

    // Load the OBJ files
    bool cBoard = loadAssImpLab3("Lab3/Stone_Chess_Board/12951_Stone_Chess_Board_v1_L3.obj", gchessComponents);
    bool cComps = loadAssImpLab3("Lab3/Chess/chess-mod.obj", gchessComponents);

    // Proceed iff OBJ loading is successful
    if (!cBoard || !cComps)
    {
        // Quit the program (Failed OBJ loading)
        std::cout << "Program failed due to OBJ loading failure, please CHECK!" << std::endl;
        return -1;
    }

    // Setup the Chess board locations
    tModelMap cTModelMap;
    setupChessBoard(cTModelMap);

    // Load it into a VBO (One time activity)
    // Run through all the components for rendering
    for (auto cit = gchessComponents.begin(); cit != gchessComponents.end(); cit++)
    {
        // Setup VBO buffers
        cit->setupGLBuffers();
        // Setup Texture
        cit->setupTextureBuffers();
    }

    // Use our shader (Not changing the shader per chess component)
    glUseProgram(programID);

    // Get a handle for our "LightPosition" uniform
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    // Thread for handling user input in parallel with game running
std::thread inputThread([&]() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(gameStateMutex);
            if (gameOver) break;
        }

        std::string input;
        std::cout << "Please enter a command: ";
        std::getline(std::cin, input);
        std::vector<float> cmd = isValidCommand(input);

        if (cmd[0] == 1) { //camera
            {
                std::lock_guard<std::mutex> lock(cameraMutex);
                thetaCamera = cmd[1];
                phiCamera = cmd[2];
                RCamera = cmd[3];
            }
            {
                std::lock_guard<std::mutex> lock(gameStateMutex);
                playerInput = 1;
            }
        }
        if (cmd[0] == 2) { //light
            {
                std::lock_guard<std::mutex> lock(lightMutex);
                thetaLight = cmd[1];
                phiLight = cmd[2];
                RLight = cmd[3];
            }
            {
                std::lock_guard<std::mutex> lock(gameStateMutex);
                playerInput = 2;
            }
        }
        if (cmd[0] == 3) { //move command
            size_t spacePos = input.find(' ');
            std::string str = input.substr(spacePos + 1);
            if (engine.isValidUCI(str)) {
                engine.sendMove(str);
                engine.getResponseMove(str);
                {
                    std::lock_guard<std::mutex> lockMove(playerOneCommandMutex);
                    playerOneCommand = str;
                }
                {
                    std::lock_guard<std::mutex> lockOpponent(playerTwoCommandMutex);
                    playerTwoCommand = engine.engineMove;
                }
                {
                    std::lock_guard<std::mutex> lock(gameStateMutex);
                    playerInput = 3;
                }
            }
            else {
                std::cout << "Invalid Command!\nINPUT ERROR: Move command must have valid UCI input\n";
            }
        }

        if (input.find("quit") == 0) {
            std::lock_guard<std::mutex> lock(gameStateMutex);
            gameOver = true;
        }
    }
});

    // Setup loop for matrixModels buffer and corresponding positions
    for (auto cit = gchessComponents.begin(); cit != gchessComponents.end(); ++cit)
    {
        size_t componentIndex = std::distance(gchessComponents.begin(), cit);
        tPosition cTPosition = cTModelMap[cit->getComponentID()];

        for (unsigned int pit = 0; pit < cTPosition.rCnt; ++pit)
        {
            // Modify cTPosition for this instance
            tPosition cTPositionMorph = cTPosition;
            cTPositionMorph.tPos.x += pit * cTPosition.rDis * CHESS_BOX_SIZE;

            // Generate the ModelMatrix and store it
            glm::mat4 ModelMatrix = cit->genModelMatrix(cTPositionMorph);
            matrixModels.push_back(ModelMatrix);

            // Store the modified position corresponding to the matrix
            positions.push_back(cTPositionMorph);

            // Map the component index to this model matrix
            components.push_back(componentIndex);
        }
    }

    int index = -1;
    int index2 = -1;
do {
    double currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 1.0) {
        nbFrames = 0;
        lastTime += 1.0;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Check player input with mutex protection
    {
        std::lock_guard<std::mutex> lock(gameStateMutex);
        if (playerInput == 1) {
            std::lock_guard<std::mutex> camLock(cameraMutex);
            setMatricesFromInputs(thetaCamera, phiCamera, RCamera);
            playerInput = 0;
        }
        else if (playerInput == 2) {
            std::lock_guard<std::mutex> lightLock(lightMutex);
            lightX = RLight * sin(glm::radians(thetaLight)) * cos(glm::radians(phiLight));
            lightY = RLight * sin(glm::radians(thetaLight)) * sin(glm::radians(phiLight));
            lightZ = RLight * cos(glm::radians(thetaLight));
            playerInput = 0;
        }
        else {
            computeMatricesFromInputsLab3();
        }
    }

    // Rest of the rendering code...

    // Check game over condition
    {
        std::lock_guard<std::mutex> lock(gameStateMutex);
        if (gameOver) {
            break;
        }
    }
} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && 
         glfwWindowShouldClose(window) == 0);

    //Join our thread that reads inputs
    inputThread.join();

    // Cleanup VBO, Texture (Done in class destructor) and shader 
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    // Shake hand for exit!
    return 0;
}

void setupChessBoard(tModelMap& cTModelMap)
{
    // Target spec Hash
    cTModelMap =
    {
        // Chess board              Count  rDis Angle      Axis             Scale                          Position (X, Y, Z)
        {"12951_Stone_Chess_Board", {1,    0,   0.f,    {1, 0, 0},    glm::vec3(CBSCALE), {0.f,     0.f,                             PHEIGHT}}},
        // First player             Count  rDis Angle      Axis             Scale                          Position (X, Y, Z)
        {"TORRE3",                  {2,   (8 - 1),90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-3.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}}},
        {"Object3",                 {2,   (6 - 1),90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-2.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}}},
        {"ALFIERE3",                {2,   (4 - 1),90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-1.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}}},
        {"REGINA2",                 {1,    0,   90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-0.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}}}, //Queen
        {"RE2",                     {1,    0,   90.f,   {1, 0, 0},    glm::vec3(CPSCALE), { 0.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}}}, //King
        {"PEDONE13",                {8,    1,   90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-3.5 * CHESS_BOX_SIZE, -2.5 * CHESS_BOX_SIZE, PHEIGHT}}} //pawns
    };

    // Second player derived from first player!!
    // Second Player (TORRE02)
    cTModelMap["TORRE02"] = cTModelMap["TORRE3"];
    cTModelMap["TORRE02"].tPos.y = -cTModelMap["TORRE3"].tPos.y;
    // Second Player (Object02)
    cTModelMap["Object02"] = cTModelMap["Object3"];
    cTModelMap["Object02"].tPos.y = -cTModelMap["Object3"].tPos.y;
    // Second Player (ALFIERE02)
    cTModelMap["ALFIERE02"] = cTModelMap["ALFIERE3"];
    cTModelMap["ALFIERE02"].tPos.y = -cTModelMap["ALFIERE3"].tPos.y;
    // Second Player (REGINA01)
    cTModelMap["REGINA01"] = cTModelMap["REGINA2"];
    cTModelMap["REGINA01"].tPos.y = -cTModelMap["REGINA2"].tPos.y;
    // Second Player (RE01)
    cTModelMap["RE01"] = cTModelMap["RE2"];
    cTModelMap["RE01"].tPos.y = -cTModelMap["RE2"].tPos.y;
    // Second Player (PEDONE12)
    cTModelMap["PEDONE12"] = cTModelMap["PEDONE13"];
    cTModelMap["PEDONE12"].tPos.y = -cTModelMap["PEDONE13"].tPos.y;
}
