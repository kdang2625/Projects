/* Author: Kenny Dang
Class: ECE4122
Last Date Modified: 9/30/2024 
Description:  Header file for centipede class
*/




#ifndef ECE_CENTIPEDE_H
#define ECE_CENTIPEDE_H

#include <SFML/Graphics.hpp>
#include <list>

class ECE_Centipede {
public:
    ECE_Centipede(float startX, float startY, bool isHead);

    void setTexture(sf::Texture& texture);

    sf::Sprite getSprite();

    void move(float offset);

    void updatePosition();

    sf::Vector2f getPosition();

    void moveDown(float offset);


    int getDirection();

    void setDirection(int d);



private:
    sf::Sprite sprite;
    sf::Vector2f position;
    int direction;
    int isHead;
};

#endif // ECE_CENTIPEDE_H
