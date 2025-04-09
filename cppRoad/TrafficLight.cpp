#include "TrafficLight.h"
#include "Car.h"
#include <chrono>
#include <thread>
#include <iostream>

TrafficLight::TrafficLight(const std::vector<std::vector<Car>>& lanesRead) : lanesRead(lanesRead) {
    laneStates = {
        {"0r", LightColor::RED},
        {"0s", LightColor::RED},
        {"1s", LightColor::RED},
        {"2l", LightColor::RED},
        {"3r", LightColor::RED},
        {"3s", LightColor::RED},
        {"4s", LightColor::RED},
        {"4l", LightColor::RED},
        {"5s", LightColor::RED},
        {"5r", LightColor::RED},
        {"6s", LightColor::RED},
        {"7l", LightColor::RED},
        {"8s", LightColor::RED},
        {"8r", LightColor::RED},
        {"9s", LightColor::RED},
        {"9l", LightColor::RED},
    };
}

void TrafficLight::run() {
    

    int patternIndex = 0;
    std::vector<LightPattern> patterns = {
        LightPattern::EASTWEST,
        LightPattern::NORTHBOUND,
        LightPattern::SOUTHBOUND,
        LightPattern::LEFTTURN,
    };

    while (true) {
        // simplified maxPressure alg, more like "weighted timing"
        LightPattern currentPattern = patterns[patternIndex];
        setPattern(currentPattern);

        int waitingCars = countCarsForPattern(currentPattern);
        int greenTime = std::clamp(4 + waitingCars * 0.1, 3.0, 10.0);

        switch (currentPattern) {
        case LightPattern::EASTWEST:
            std::cout << "eastwest : " << greenTime << std::endl;
            break;
        case LightPattern::SOUTHBOUND:
            std::cout << "southbound : " << greenTime << std::endl;
            break;
        case LightPattern::NORTHBOUND:
            std::cout << "northbound : " << greenTime << std::endl;
            break;
        case LightPattern::LEFTTURN:
            std::cout << "leftturn : " << greenTime << std::endl;
            break;
        }


        {
            std::lock_guard<std::mutex> lock(lightMutex);
            laneStatesRead = laneStates;
        }

        std::this_thread::sleep_for(std::chrono::seconds(greenTime));

        for (auto& [lane, color] : laneStates) {
            if (color == LightColor::GREEN) {
                color = LightColor::YELLOW;
            }
        }

        {
            std::lock_guard<std::mutex> lock(lightMutex);
            laneStatesRead = laneStates;
        }

        std::cout << "yellow" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));

        for (auto& [lane, color] : laneStates) {
            if (color == LightColor::YELLOW) {
                color = LightColor::RED;
            }
        }

        {
            std::lock_guard<std::mutex> lock(lightMutex);
            laneStatesRead = laneStates;
        }
        std::cout << "red" << std::endl;

        // buffer to let all cars pass
        std::this_thread::sleep_for(std::chrono::seconds(2));

        patternIndex = (patternIndex + 1) % patterns.size();
    }
}


LightColor TrafficLight::getState(std::string lane) {
    std::lock_guard<std::mutex> lock(lightMutex);
    return laneStatesRead[lane];
}

std::unordered_map<std::string, LightColor> TrafficLight::getAllStates() {
    std::lock_guard<std::mutex> lock(lightMutex); 
    return laneStatesRead;
}

void TrafficLight::setPattern(LightPattern pat) {
    // define wchich lanes should be green for each pattern
    std::vector<std::string> greenLanes;

    if (pat == LightPattern::NORTHBOUND) {
        greenLanes = { "3r", "3s", "4s", "4l", "5r"};
    }
    else if (pat == LightPattern::SOUTHBOUND) {
        greenLanes = { "8r", "8s", "9s", "9l", "0r" };
    }
    else if (pat == LightPattern::EASTWEST) {
        greenLanes = { "0s", "0r", "1s", "5r", "5s", "6s" };
    }
    else { // LEFTTURN
        greenLanes = { "2l", "0r", "3r", "5r", "7l", "8r" };
    }

    // set the specific lanes to GREEN
    for (const std::string& lane : greenLanes) {
        laneStates[lane] = LightColor::GREEN;
    }
}


int TrafficLight::countCarsForPattern(LightPattern pattern) {
    std::unordered_map<LightPattern, std::vector<int>> incomingLanes = {
            {LightPattern::NORTHBOUND, {3, 4}},
            {LightPattern::SOUTHBOUND, {8, 9}},
            {LightPattern::EASTWEST, {0, 1, 5, 6}},
            {LightPattern::LEFTTURN, {7, 2}},
    };
    int incoming = 0;

    for (int lane : incomingLanes[pattern]) {
        incoming += lanesRead[lane].size();
    }

    return incoming;
}