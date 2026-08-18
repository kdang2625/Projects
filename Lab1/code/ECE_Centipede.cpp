/* Author: Kenny Dang
Class: ECE4122
Last Date Modified: 9/30/2024 
Description:  Takes care of centipede behavior
*/



#include "ECE_Centipede.h"

ECE_Centipede::ECE_Centipede(float x, float y, bool h) {
    position.x = x;
    position.y = y;

    direction = 1;

    isHead = h;

    sprite.setPosition(position);
}

void ECE_Centipede::setTexture(sf::Texture& texture) {
    sprite.setTexture(texture);
    sprite.setScale(0.9, 0.9);
}

sf::Sprite ECE_Centipede::getSprite() {
    return sprite;
}

void ECE_Centipede::move(float offset) {
    sprite.move(offset, 0);
    position = sprite.getPosition();
}

void ECE_Centipede::setDirection(int d) {
    direction = d;
}

void ECE_Centipede::moveDown(float offset) {
    sprite.move(0.0, offset);
    direction = -direction;
    position = sprite.getPosition();
}

void ECE_Centipede::updatePosition() {
    position = sprite.getPosition();
}

sf::Vector2f ECE_Centipede::getPosition() {
    return position;
}

int ECE_Centipede::getDirection() {
    return direction;
}