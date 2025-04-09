#include <SFML/Graphics.hpp>
#include "Car.h"
#include <iostream>
#include <unordered_set>

Car::Car(int lane, char turnDirection, int maxSpeed, int acceleration) : lane(lane), turnDirection(turnDirection), maxSpeed(maxSpeed), acceleration(acceleration) {
	shape.setSize({ 10, 10 });
	shape.setFillColor(sf::Color::Blue);
	shape.setPosition(position);
	initPosDir();
	speed = maxSpeed;
}


void Car::updatePos(float deltaTime, const Car* carAhead, bool isGreen, bool isOut) {
	constexpr float PI = 3.14159265f;
	constexpr float minGap = 20.0f;
	int breakDist = ((speed * speed) / (2 * acceleration));

	auto shouldBrake = [&](bool condition) {
		if (condition) {
			speed = std::max(0.0f, speed - acceleration * deltaTime);
		}
		else {
			speed = std::min(maxSpeed, speed + acceleration * deltaTime);
		}
	};


	if (isOut) {
		speed = std::min(maxSpeed, speed + acceleration * deltaTime);
	}
	else if (!isGreen && carAhead == nullptr) {

		// break at lights
		switch (startDirection) {
		case Direction::West:
			shouldBrake(position.x - breakDist <= 500);
			break;
		case Direction::East:
			shouldBrake(position.x + breakDist >= 300);
			break;
		case Direction::North:
			shouldBrake(position.y - breakDist <= 530);
			break;
		case Direction::South:
			shouldBrake(position.y + breakDist >= 270);
			break;
		}
	}
	else if (!isGreen && carAhead != nullptr) {
		int aheadBreakDist = (carAhead->speed * carAhead->speed) / (2 * carAhead->acceleration);
		float carAheadX = carAhead->position.x;
		float carAheadY = carAhead->position.y;

		// break at lights or break earlier if car ahead will get too close
		switch (startDirection) {
		case Direction::West:
			shouldBrake(position.x - breakDist <= 500 || position.x - breakDist <= carAheadX - aheadBreakDist + minGap);
			break;
		case Direction::East:
			shouldBrake(position.x + breakDist >= 300 || position.x + breakDist >= carAheadX + aheadBreakDist - minGap);
			break;
		case Direction::North:
			shouldBrake(position.y - breakDist <= 530 || position.y - breakDist <= carAheadY - aheadBreakDist + minGap);
			break;
		case Direction::South:
			shouldBrake(position.y + breakDist >= 270 || position.y + breakDist >= carAheadY + aheadBreakDist - minGap);
			break;
		}
	}
	else if (isGreen && carAhead != nullptr) {
		int aheadBreakDist = (carAhead->speed * carAhead->speed) / (2 * carAhead->acceleration);
		float carAheadX = carAhead->position.x;
		float carAheadY = carAhead->position.y;

		// follow car ahead
		switch (startDirection) {
		case Direction::West:
			shouldBrake(position.x - breakDist <= carAheadX - aheadBreakDist + minGap);
			break;
		case Direction::East:
			shouldBrake(position.x + breakDist >= carAheadX + aheadBreakDist - minGap);
			break;
		case Direction::North:
			shouldBrake(position.y - breakDist <= carAheadY - aheadBreakDist + minGap);
			break;
		case Direction::South:
			shouldBrake(position.y + breakDist >= carAheadY + aheadBreakDist - minGap);
			break;
		}
	}
	else {
		// no car in the way and light is green so go
		speed = std::min(maxSpeed, speed + acceleration * deltaTime);
	}
	
	// movement patterns based on the lane, turnDirection and progress
	switch (lane) { 
	case 0:
		if (turnDirection == 's') {
			position.x -= speed * deltaTime;
			break;
		}

		if (turnDirection == 'r' && !isTurning && position.x < 490 && position.x > 441) {
			// start turn
			isTurning = true;
			turnCenter = sf::Vector2f(490, 270);
			turnProgress = 0.0f;
		}

		if (isTurning) {
			float turnRadius = 50.0f;
			float angularSpeed = speed / turnRadius;
			turnProgress += angularSpeed * deltaTime;

			// circle equations
			position.x = turnCenter.x - turnRadius * sin(turnProgress);
			position.y = turnCenter.y + turnRadius * cos(turnProgress);

			if (turnProgress >= PI / 2) {
				isTurning = false; // end of turn
			}
		}
		else if (position.x >= 490) {
			// original straight left movement
			position.x -= speed * deltaTime;
		}
		else {
			// driving away from intersection
			position.y -= speed * deltaTime;
		}
		break;

	case 1:
		position.x -= speed * deltaTime;
		break;

	case 2:
		if (turnDirection == 'l' && !isTurning && position.x < 490 && position.x > 381) {
			// start turn
			isTurning = true;
			turnCenter = sf::Vector2f(490, 520);
			turnProgress = 0.0f;
		}

		if (isTurning) {
			float turnRadiusX = 110.0f;
			float turnRadiusY = 140.0f;
			// only an approx of angular speed for simplicity
			float angularSpeed = speed / sqrt(
				pow(turnRadiusX * sin(turnProgress), 2) +
				pow(turnRadiusY * cos(turnProgress), 2)
			);
			turnProgress += angularSpeed * deltaTime;

			// circle equations
			position.x = turnCenter.x - turnRadiusX * sin(turnProgress);
			position.y = turnCenter.y - turnRadiusY * cos(turnProgress);

			if (turnProgress >= PI / 2) {
				isTurning = false; // end of turn
			}
		}
		else if (position.x >= 490) {
			// original straight movement
			position.x -= speed * deltaTime;
		}
		else {
			// driving away from intersection
			position.y += speed * deltaTime;
		}
		break;

	case 3:
		if (turnDirection == 's') {
			position.y -= speed * deltaTime;
			break;
		}

		if (turnDirection == 'r' && !isTurning && position.y < 520 && position.y > 471) {
			// start turn
			isTurning = true;
			turnCenter = sf::Vector2f(490, 520);
			turnProgress = 0.0f;
		}

		if (isTurning) {
			float turnRadius = 50.0f;
			float angularSpeed = speed / turnRadius;
			turnProgress += angularSpeed * deltaTime;

			// circle equations
			position.x = turnCenter.x - turnRadius * cos(turnProgress);
			position.y = turnCenter.y - turnRadius * sin(turnProgress);

			if (turnProgress >= PI / 2) {
				isTurning = false; // end of turn
			}
		}
		else if (position.y >= 520) {
			// original straight movement
			position.y -= speed * deltaTime;
		}
		else {
			// driving away from intersection
			position.x += speed * deltaTime;
		}
		break;

	case 4:
		if (turnDirection == 's') {
			position.y -= speed * deltaTime;
			break;
		}

		if (turnDirection == 'l' && !isTurning && position.y < 520 && position.y > 351) {
			// start turn
			isTurning = true;
			turnCenter = sf::Vector2f(300, 520);
			turnProgress = 0.0f;
		}

		if (isTurning) {
			float turnRadiusX = 110.0f;
			float turnRadiusY = 170.0f;

			// only an approx of angular speed for simplicity
			float angularSpeed = speed / sqrt(
				pow(turnRadiusX * sin(turnProgress), 2) +
				pow(turnRadiusY * cos(turnProgress), 2)
			);
			turnProgress += angularSpeed * deltaTime;

			// circle equations
			position.x = turnCenter.x + turnRadiusX * cos(turnProgress);
			position.y = turnCenter.y - turnRadiusY * sin(turnProgress);

			if (turnProgress >= PI / 2) {
				isTurning = false; // end of turn
			}
		}
		else if (position.y >= 520) {
			// original straight movement
			position.y -= speed * deltaTime;
		}
		else {
			// driving away from intersection
			position.x -= speed * deltaTime;
		}
		break;

	case 5:
		if (turnDirection == 's') {
			position.x += speed * deltaTime;
			break;
		}

		if (turnDirection == 'r' && !isTurning && position.x < 349 && position.x > 299) {
			// start turn
			isTurning = true;
			turnCenter = sf::Vector2f(300, 520);
			turnProgress = 0.0f;
		}

		if (isTurning) {
			float turnRadius = 50.0f;
			float angularSpeed = speed / turnRadius;
			turnProgress += angularSpeed * deltaTime;

			// circle equations
			position.x = turnCenter.x + turnRadius * sin(turnProgress);
			position.y = turnCenter.y - turnRadius * cos(turnProgress);

			if (turnProgress >= PI / 2) {
				isTurning = false; // end of turn
			}
		}
		else if (position.x <= 299) {
			// original straight left movement
			position.x += speed * deltaTime;
		}
		else {
			// driving away from intersection
			position.y += speed * deltaTime;
		}
		break;

	case 6:
		position.x += speed * deltaTime;
		break;

	case 7:
		if (turnDirection == 'l' && !isTurning && position.x < 409 && position.x > 310) {
			// start turn
			isTurning = true;
			turnCenter = sf::Vector2f(310, 280);
			turnProgress = 0.0f;
		}

		if (isTurning) {
			float turnRadiusX = 100.0f;
			float turnRadiusY = 130.0f;
			// only an approx of angular speed for simplicity
			float angularSpeed = speed / sqrt(
				pow(turnRadiusX * sin(turnProgress), 2) +
				pow(turnRadiusY * cos(turnProgress), 2)
			);
			turnProgress += angularSpeed * deltaTime;

			// circle equations
			position.x = turnCenter.x + turnRadiusX * sin(turnProgress);
			position.y = turnCenter.y + turnRadiusY * cos(turnProgress);

			if (turnProgress >= PI / 2) {
				isTurning = false; // end of turn
			}
		}
		else if (position.x <= 310) {
			// original straight movement
			position.x += speed * deltaTime;
		}
		else {
			// driving away from intersection
			position.y -= speed * deltaTime;
		}
		break;

	case 8:
		if (turnDirection == 's') {
			position.y += speed * deltaTime;
			break;
		}

		if (turnDirection == 'r' && !isTurning && position.y < 319 && position.y > 279) {
			// start turn
			isTurning = true;
			turnCenter = sf::Vector2f(310, 280);
			turnProgress = 0.0f;
		}

		if (isTurning) {
			float turnRadius = 40.0f;
			float angularSpeed = speed / turnRadius;
			turnProgress += angularSpeed * deltaTime;

			// circle equations
			position.x = turnCenter.x + turnRadius * cos(turnProgress);
			position.y = turnCenter.y + turnRadius * sin(turnProgress);

			if (turnProgress >= PI / 2) {
				isTurning = false; // end of turn
			}
		}
		else if (position.y < 280) {
			// original straight left movement
			position.y += speed * deltaTime;
		}
		else {
			// driving away from intersection
			position.x -= speed * deltaTime;
		}
		break;

	case 9:
		if (turnDirection == 's') {
			position.y += speed * deltaTime;
			break;
		}

		if (turnDirection == 'l' && !isTurning && position.y < 439 && position.y > 279) {
			// start turn
			isTurning = true;
			turnCenter = sf::Vector2f(480, 280);
			turnProgress = 0.0f;
		}

		if (isTurning) {
			float turnRadiusX = 100.0f;
			float turnRadiusY = 160.0f;
			// only an approx of angular speed for simplicity
			float angularSpeed = speed / sqrt(
				pow(turnRadiusX * sin(turnProgress), 2) +
				pow(turnRadiusY * cos(turnProgress), 2)
			);
			turnProgress += angularSpeed * deltaTime;

			// circle equations
			position.x = turnCenter.x - turnRadiusX * cos(turnProgress);
			position.y = turnCenter.y + turnRadiusY * sin(turnProgress);

			if (turnProgress >= PI / 2) {
				isTurning = false; // end of turn
			}
		}
		else if (position.y <= 280) {
			// original straight movement
			position.y += speed * deltaTime;
		}
		else {
			// driving away from intersection
			position.x += speed * deltaTime;
		}
		break;
	}

	//apply changes
	shape.setPosition(position);
}


void Car::initPosDir() {
	switch (lane) {
	case 0:
		position = { 910 , 320 };
		startDirection = Direction::West;
		break;
	case 1:
		position = { 910, 350 };
		startDirection = Direction::West;
		break;
	case 2:
		position = { 910, 380 };
		startDirection = Direction::West;
		break;
	case 3:
		position = { 440, 910 };
		startDirection = Direction::North;
		break;
	case 4:
		position = { 410, 910 };
		startDirection = Direction::North;
		break;
	case 5:
		position = { -110, 470 };
		startDirection = Direction::East;
		break;
	case 6:
		position = { -110, 440 };
		startDirection = Direction::East;
		break;
	case 7:
		position = { -110, 410 };
		startDirection = Direction::East;
		break;
	case 8:
		position = { 350, -110 };
		startDirection = Direction::South;
		break;
	case 9:
		position = { 380, -110 };
		startDirection = Direction::South;
		break;
	}
}


void Car::draw(sf::RenderWindow& window) {
	window.draw(shape);
}

const sf::Vector2f& Car::getPosition() const {
	return position;
}

const sf::RectangleShape& Car::getShape() const {
	return shape;
}

char Car::getTurnDir() const {
	return turnDirection;
}

float Car::getSpeed() const {
	return speed;
}

Direction Car::getStartDirection() const {
	return startDirection;
}
