/* Author: Kenny Dang
Class: ECE4122
Last Date Modified: 9/30/2024 
Description:  Header file for the laserblast
*/





#ifndef ECE_LASER_BLAST_H
#define ECE_LASER_BLAST_H

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

class ECE_LaserBlast {
public:
    ECE_LaserBlast(sf::Vector2f start);
    void updateSpeed(float time);
    sf::FloatRect getGlobalBounds() const;
    bool outWindow(float screenHeight);
    void draw(sf::RenderWindow& window) const;
    void laserMove();

private:
    sf::RectangleShape laser;
    sf::Vector2f speed;
    sf::Clock gameClock;
};

#endif // ECE_LASER_BLAST_H
