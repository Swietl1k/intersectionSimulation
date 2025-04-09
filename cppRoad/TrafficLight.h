#ifndef LIGHT_H
#define LIGHT_H

#include <SFML/Graphics.hpp>
#include <mutex>
#include <condition_variable>
#include <vector>
#include "Car.h"

enum class LightColor {
	GREEN,
	RED,
	YELLOW,
};

enum class LightPattern {
	NORTHBOUND,
	SOUTHBOUND,
	EASTWEST,
	LEFTTURN,
};

class TrafficLight {
public:
	TrafficLight(const std::vector<std::vector<Car>>& lanesRead);
	void run();
	void draw(sf::RenderWindow& window);
	LightColor getState(std::string lane);
	std::unordered_map<std::string, LightColor> getAllStates();


private:
	void updateLights();
	void setPattern(LightPattern pattern);
	int countCarsForPattern(LightPattern pattern);

	const std::vector<std::vector<Car>>& lanesRead;
	std::mutex lightMutex;
	std::unordered_map<std::string, LightColor> laneStates;
	std::unordered_map<std::string, LightColor> laneStatesRead;
};

#endif
