/* Author: Kenny Dang
Class: ECE4122
Last Date Modified: 9/30/2024 
Description:  Laser blast class to execute the behavior of the laser
*/



#include "ECE_LaserBlast.h"

ECE_LaserBlast::ECE_LaserBlast(sf::Vector2f start) {
    laser.setSize(sf::Vector2f(5, 5));
    laser.setFillColor(sf::Color::Red);
    laser.setPosition(start);
    speed = sf::Vector2f(0.0f, -1000.f);
}

void ECE_LaserBlast::updateSpeed(float time) {
    laser.move(speed * time);
}

sf::FloatRect ECE_LaserBlast::getGlobalBounds() const {
    return laser.getGlobalBounds();
}

bool ECE_LaserBlast::outWindow(float screenHeight) {
    return laser.getPosition().y > screenHeight;
}

void ECE_LaserBlast::draw(sf::RenderWindow& window) const {
    window.draw(laser);
}

void ECE_LaserBlast::laserMove() {
    laser.setPosition(-100, -100);
}
