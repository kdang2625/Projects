/* Author: Kenny Dang
Class: ECE4122
Last Date Modified: 9/30/2024 
Description:  Main file used to run the game loop.
*/


// Include important C++ libraries here
#include <sstream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "ECE_Centipede.h"
#include "ECE_LaserBlast.h"
#include <iostream>
#include <random>
#include <string>
#include <list>
#include <iterator>


// Make code easier to type with "using namespace"
using namespace sf;
// Function declaration



struct shrooms {
	Sprite sprite;
	int lives = 0;
};

int main()
{
	// Create a video mode object
	VideoMode vm(1240, 720);

	// Create and open a window for the game
	RenderWindow window(vm, "ECE_Centipede", Style::Default);
	// FPS limit so it runs on PACE ICE better
	window.setFramerateLimit(45);

	// Create a texture to hold a graphic on the GPU
	Texture textureBackground;

	// Load a graphic into the texture
	textureBackground.loadFromFile("graphics/StartupScreenBackground.png");
	// Create a sprite
	Sprite spriteBackground;
	// Gameclock to allow for movement relative to time
	Clock gameClock;
	
	// Attach the texture to the sprite
	spriteBackground.setTexture(textureBackground);

	// Set the spriteBackground to cover the screen
	spriteBackground.setPosition(0, 0);
	spriteBackground.scale(1.5,1.5);

	// Make a mushroom sprite
	Texture mushroom0;
	mushroom0.loadFromFile("graphics/Mushroom0.png");
	Sprite fullMushroom;
	fullMushroom.setTexture(mushroom0);

	Texture mushroom1;
	mushroom1.loadFromFile("graphics/Mushroom1.png");
	Sprite halfMushroom;
	halfMushroom.setTexture(mushroom1);
	//*******************************************************
	// reloading centipede texture in case the class fails to do so
	sf::Texture centipedeHeadTexture;
	centipedeHeadTexture.loadFromFile("graphics/CentipedeHead.png");

	float startingPosition = 450.0f;
	// implementing the constructor to create the centipede
	ECE_Centipede centipedeHead = ECE_Centipede(startingPosition, 30.0f, true);
	centipedeHead.setTexture(centipedeHeadTexture);
	startingPosition -= 35.0f;
	
	std::vector<ECE_Centipede> centipede;
	centipede.push_back(centipedeHead);

	sf::Texture centipedeBodyTexture;
	centipedeBodyTexture.loadFromFile("graphics/CentipedeBody.png");

	for (int i = 0; i < 11; i++) 
	{
		ECE_Centipede temp = ECE_Centipede(startingPosition, 30.0f, false);
		temp.setTexture(centipedeBodyTexture);
		startingPosition -= 35.0f;
		centipede.push_back(temp);
	}


//**************************************************************************
	// initializations
	bool startOfGame = true;
	bool acceptInput = false;
	int playerScore = 0;
	
//*****************************************************************
	// setup Spaceship
	Texture spaceshipTexture;
	spaceshipTexture.loadFromFile("graphics/StarShip.png");
	Sprite spaceshipSprite;
	spaceshipSprite.setTexture(spaceshipTexture);
	float shipWidth = spaceshipTexture.getSize().x;
	float shipHeight = spaceshipTexture.getSize().y;
	float shipXStart = 650.0f;
	float shipYStart = 650.0f;
	spaceshipSprite.setPosition(shipXStart, shipYStart);
	float shipSpeed = 10.0f;
	int playerLives = 3;

	// drawing lives like in the real game
	Sprite lifeOne;
	Sprite lifeTwo;
	Sprite lifeThree;
	lifeOne.setTexture(spaceshipTexture);
	lifeTwo.setTexture(spaceshipTexture);
	lifeThree.setTexture(spaceshipTexture);
	lifeOne.setPosition(800,25);
	lifeTwo.setPosition(850, 25);
	lifeThree.setPosition(900, 25);


//*******************************************************************

	// spider initilizations
	Texture spiderTexture;
	spiderTexture.loadFromFile("graphics/spider.png");
	Sprite spiderSprite;
	spiderSprite.setTexture(spiderTexture);
	spiderSprite.setPosition(200.0f, 200.0f);
	bool spiderMoving = false;
	float spiderVelocity = 0.0f;


	//set up the mushrooms
	std::vector<shrooms> shroom;
	//random number generator
	std::random_device rand;
	std::mt19937 gen(rand());
	std:: uniform_int_distribution<> xD(150, 1100);
	std:: uniform_int_distribution<> yD(100, 500);
	int num_mushrooms = 30;
	// for loop to print the mushrooms randomly
	for (int i = 0; i < num_mushrooms; i++) 
	{
		shrooms mushroom;
		mushroom.lives = 0;
		mushroom.sprite.setTexture(mushroom0);
		float mushroomX = xD(gen);
		float mushroomY = yD(gen);
		mushroom.sprite.setPosition(mushroomX, mushroomY);
		shroom.push_back(mushroom);
	}

	//*******************************************************************
	//Setting up Laser
	RectangleShape laserBlast;
	laserBlast.setSize(sf::Vector2f(10,50));
	laserBlast.setFillColor(Color::Red);
	laserBlast.setPosition(20, 50);
	std::vector<ECE_LaserBlast> spriteLaserBlasts;
	float timeLaser = 0.0f;
	float delay = 0.1f;

	// Setting up Text and Score
	sf::Text messageText;
	sf::Text scoreText;
	sf::Font font;
	font.loadFromFile("fonts/KOMIKAP_.ttf");
	// Set the font to our message
	messageText.setFont(font);
	scoreText.setFont(font);
	scoreText.setCharacterSize(50);
	scoreText.setFillColor(Color::White);
	scoreText.setPosition(20, 20);



	while (window.isOpen())
	{
	
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::KeyReleased && !startOfGame)
			{
				acceptInput = true;
			}
		}

		/*
		****************************************
		Handle the players input
		****************************************
		*/

		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}
		// Start the games
		if (Keyboard::isKeyPressed(Keyboard::Return))
		{
			startOfGame = true;
			window.clear();
			acceptInput = true;
		}
		float dtTime = gameClock.restart().asSeconds();
		// *********************************
		// Player Movement *****************
		// *********************************
		if (acceptInput)
		{
			//Handle the right cursor key
			timeLaser = timeLaser + dtTime;
			if (Keyboard::isKeyPressed(Keyboard::Right))
			{
				spaceshipSprite.move(shipSpeed, 0.0f);
			}

			// Handle the left cursor key
			if (Keyboard::isKeyPressed(Keyboard::Left))
			{
				spaceshipSprite.move(-shipSpeed, 0.0f);
			}

			// Handle the up cursor key
			if (Keyboard::isKeyPressed(Keyboard::Up))
			{
				spaceshipSprite.move(0.0f, -shipSpeed);			
			}

			// Handle the down cursor key
			if (Keyboard::isKeyPressed(Keyboard::Down))
			{
				spaceshipSprite.move(0.0f, shipSpeed);		
			}
			// draws backround before the game starts
			if (startOfGame) {
			window.draw(spriteBackground);
			}

			// Laser delay so that the laser is not constantly shooting
			if (timeLaser >= delay && Keyboard::isKeyPressed(Keyboard::Space)) 
			{
				ECE_LaserBlast laser(Vector2f(spaceshipSprite.getPosition().x, spaceshipSprite.getPosition().y));
				spriteLaserBlasts.push_back(laser);
				timeLaser = 0.0f;
			}

			//updates laser movement
			for (auto laser = spriteLaserBlasts.begin(); laser != spriteLaserBlasts.end(); ) 
			{
				laser->updateSpeed(dtTime); // moves laser relative to time
				// if the laser leaves the window, delete it
				if (laser->outWindow(vm.height)) {
					laser = spriteLaserBlasts.erase(laser);
				} else {
					laser++;
				}

				// Laser intersects spider
				if (laser -> getGlobalBounds().intersects(spiderSprite.getGlobalBounds())) 
				{
					playerScore = playerScore + 300; //Spider intersect score
					spiderSprite.setPosition(500,500);
				}
				
			}

			// laser intersects mushrooms
			for (auto& laser : spriteLaserBlasts) 
			{ // for each laser in the vector
				for (shrooms& i : shroom) 
				{ // for each laser mushroom in the mushroom list
					if (laser.getGlobalBounds().intersects(i.sprite.getGlobalBounds())) 
					{ // increment the life of the mushroom to change the png
						i.lives++;
						if (i.lives == 1) 
						{
							i.sprite.setTexture(mushroom1);
						} else if (i.lives == 2) 
						{
							i.sprite.setPosition(-100, -100); 
							playerScore += 4;
						}
						laser.laserMove();
					}
				}
			}
		}

		
		// for each segment of the centipede, move down when it hits a bound
		for (auto& c : centipede) 
		{
			if (c.getPosition().x <= 0.0f) c.moveDown(30.0f);
			if (c.getPosition().x >= 1240.0f) c.moveDown(30.0f);
			c.move(c.getDirection() * 5.0f);
			c.updatePosition();
		}
		
		
		/*
		****************************************
		Update the scene
		****************************************
		*/
		window.clear();
		window.draw(spriteBackground);

		if (!startOfGame) 
		{ //print loop
			Time dt = gameClock.restart();
			float dtTime = gameClock.restart().asSeconds();
			window.clear();
			window.draw(spaceshipSprite);
			window.draw(lifeOne);
			window.draw(lifeTwo);
			window.draw(lifeThree);
			window.draw(scoreText);
			window.draw(spiderSprite);
			for (shrooms& s : shroom) 
			{
				window.draw(s.sprite);
			}
			for (const auto& i: spriteLaserBlasts) 
			{
                i.draw(window);
            }
			for (auto& c : centipede) 
			{
				window.draw(c.getSprite());
			}
			//updating scoretext
			std::stringstream ss;
			ss << "Score: " << playerScore;
			scoreText.setString(ss.str());

			if(!spiderMoving) 
			{ // reupdating spider
				spiderMoving = true;
				spiderVelocity = 5.0f;
				float height = (rand() % 500) + 100;
				spiderSprite.setPosition(1000, height);
			} else 
			{
				// Implemented Random Spider Movement
				int spiderDirection = (std::rand() % 4);
				int moveCounter = 0;

				if (spiderDirection == 0) { // Move left
					while (moveCounter < 5000) {
						if (spiderSprite.getPosition().x > 0.0f) 
						{
							spiderSprite.move(-spiderVelocity * dt.asSeconds(), 0.0f);
							moveCounter++; // allowing for 5000 ticks in a certain direction
						} else 
						{
							break; // Stop moving if at the left boundary
						}
					}
				} else if (spiderDirection == 1) 
				{ // Move right
					while (moveCounter < 5000) 
					{
						if (spiderSprite.getPosition().x < window.getSize().x - spiderSprite.getGlobalBounds().width) 
						{
							spiderSprite.move(spiderVelocity * dt.asSeconds(), 0.0f);
							moveCounter++;
						} else 
						{
							break; // Stop moving if at the right boundary
						}
					}
				} else if (spiderDirection == 2) 
				{ // Move down
					while (moveCounter < 5000) 
					{
						if (spiderSprite.getPosition().y < window.getSize().y - spiderSprite.getGlobalBounds().height) 
						{
							spiderSprite.move(0.0f, spiderVelocity * dt.asSeconds());
							moveCounter++;
						} else 
						{
							break; // Stop moving if at the bottom boundary
						}
					}
				} else if (spiderDirection == 3) 
				{ // Move up
					while (moveCounter < 5000) 
					{
						if (spiderSprite.getPosition().y > 0.0f) 
						{
							spiderSprite.move(0.0f, -spiderVelocity * dt.asSeconds());
							moveCounter++;
						} else 
						{
							break; // Stop moving if at the top boundary
						}
					}
				}
				// Reset moveCounter
				moveCounter = 0;
				// Handles Spaceship and Spider intersections
				if (spaceshipSprite.getGlobalBounds().intersects(spiderSprite.getGlobalBounds())) 
				{
					playerLives--; //decrements players lives
					spaceshipSprite.setPosition(shipXStart,shipYStart); // sent back to origin when hit
					if (playerLives == 2) 
					{
						lifeOne.setPosition(-100,-100); //deletes lives from screen
					} else if (playerLives == 1) 
					{
						lifeTwo.setPosition(-100,-100); //deletes lives from screen
					} else 
					{
						lifeThree.setPosition(-100,100); //deletes lives from screen
					}
				}
				for (shrooms& i : shroom) 
				{
					if (spiderSprite.getGlobalBounds().intersects(i.sprite.getGlobalBounds())) 
					{ // handles mushroom and spider interaction
						i.sprite.setPosition(-100, -100);
					}

					// contact with centipede
					for (int j = 0; j < centipede.size(); j++) 
					{ 
						if (centipede[j].getSprite().getGlobalBounds().intersects(i.sprite.getGlobalBounds())) 
						{ // collision with other sprites (mushrooms)
							centipede[j].moveDown(30.0f);
						}
					}
				}
			}



			// Game over screen
			if (playerLives == 0) 
			{
				window.clear();
				messageText.setString("Game Over !!! \nPress Escape to\nclose the window!!!");
				messageText.setCharacterSize(75);
				messageText.setPosition(20, 20);
				messageText.setFillColor(Color::White);
				window.draw(messageText);
			}

		



		}

		if (Keyboard::isKeyPressed(Keyboard::Return) && startOfGame) 
		{
			startOfGame = false;
			window.clear();
		}
		
		window.display();
	}

	return 0;
}

// Function definition
