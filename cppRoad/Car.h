#ifndef CAR_H
#define CAR_H

#include <SFML/Graphics.hpp>

enum class Direction { East, West, South, North };

class Car {
public:
	Car(int lane, char turnDirection, int maxSpeed, int acceleration);
	void updatePos(float deltaTime, const Car* carAhead, bool isRed, bool isOut);
	void draw(sf::RenderWindow& window);
	const sf::Vector2f& getPosition() const;
	const sf::RectangleShape& getShape() const;
	char getTurnDir() const;
	float getSpeed() const;
	Direction getStartDirection() const;


private:
	void initPosDir();


	sf::RectangleShape shape;
	sf::Vector2f position;
	float speed;
	float acceleration;
	float maxSpeed;
	int lane;
	char turnDirection;
	Direction startDirection;

	bool isTurning = false;
	float turnProgress = 0.0f; // 0-1 for turn completion
	sf::Vector2f turnCenter;

};

#endif 
