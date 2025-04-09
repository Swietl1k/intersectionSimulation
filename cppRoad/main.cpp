#include <iostream>
#include <vector>
#include <random>
#include <unordered_set>
#include <thread>
#include "Car.h"
#include "TrafficLight.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const float TIME_PER_UPDATE = 1.0f / 60.0f;
const int NEW_CAR_INTERVAL = 1;
const int CARS_PER_INTERVAL = 6; // cant be more than 10
const int CAR_MAX_SPEED = 60;
const int CAR_ACCELERATION = 20;
const std::unordered_set<int> eastLanes = { 0, 1, 2 };
const std::unordered_set<int> southLanes = { 3, 4 };
const std::unordered_set<int> westLanes = { 5, 6, 7 };
const std::unordered_set<int> northLanes = { 8, 9 };


int randLane() {
    static std::random_device rd;  
    static std::mt19937 gen(rd()); 
    static std::uniform_int_distribution<int> dist(0, 9); 

    return dist(gen);
}


void drawCars(sf::RenderWindow& window, std::vector<std::vector<Car>>& lanes, std::vector<std::vector<Car>>& lanesPastLight) {
    for (int i = 0; i < lanes.size(); i++) {
        for (int j = 0; j < lanes[i].size(); j++) {
            lanes[i][j].draw(window);
        }
    }

    for (int i = 0; i < lanesPastLight.size(); i++) {
        for (int j = 0; j < lanesPastLight[i].size(); j++) {
            lanesPastLight[i][j].draw(window);
        }
    }
}


void updateLanes(TrafficLight& trafficLight, std::vector<std::vector<Car>>& lanes, std::vector<std::vector<Car>>& lanesRead, std::vector<std::vector<Car>>& lanesPastLight) {
    std::unordered_map<std::string, LightColor> laneStates = trafficLight.getAllStates();

    for (int laneIdx = 0; laneIdx < lanes.size(); ++laneIdx) {
        std::vector<Car>& lane = lanes[laneIdx];
        if (lane.empty()) continue;
        Car& firstCar = lane[0];

        switch (lane[0].getStartDirection()) {
        case Direction::East:
            if (firstCar.getPosition().x > 310) {
                lanesPastLight[laneIdx].push_back(std::move(lane[0]));
                lane.erase(lane.begin());
                lanesRead = lanes;
            }
            break;
        case Direction::West:
            if (firstCar.getPosition().x < 490) {
                lanesPastLight[laneIdx].push_back(std::move(lane[0]));
                lane.erase(lane.begin());
                lanesRead = lanes;
            }
            break;
        case Direction::South:
            if (firstCar.getPosition().y > 280) {
                lanesPastLight[laneIdx].push_back(std::move(lane[0]));
                lane.erase(lane.begin());
                lanesRead = lanes;
            }
            break;
        case Direction::North:
            if (firstCar.getPosition().y < 520) {
                lanesPastLight[laneIdx].push_back(std::move(lane[0]));
                lane.erase(lane.begin());
                lanesRead = lanes;
            }
            break;
        }

        for (int carIdx = 0; carIdx < lane.size(); ++carIdx) {
            Car* pcarAhead = nullptr;
            Car& car = lane[carIdx];

            if (carIdx > 0) {
                pcarAhead = &lane[carIdx - 1];
            }

            // compose lane string key like "3r", "4s", etc
            std::string laneKey = std::to_string(laneIdx) + car.getTurnDir();
            LightColor light = laneStates[laneKey];
            bool isGreen = (light == LightColor::GREEN);

            car.updatePos(TIME_PER_UPDATE, pcarAhead, isGreen, false);
        }
    }

    for (int laneIdx = 0; laneIdx < lanesPastLight.size(); ++laneIdx) {
        std::vector<Car>& lane = lanesPastLight[laneIdx];
        if (lane.empty()) continue;
        Car& firstCar = lane[0];

        if (firstCar.getPosition().x < -150.0f || firstCar.getPosition().x > 960.0f ||
            firstCar.getPosition().y < -150.0f || firstCar.getPosition().y > 960.0f) {
            lane.erase(lane.begin());
        }

        for (int carIdx = 0; carIdx < lane.size(); ++carIdx) {
            Car& car = lane[carIdx];
            car.updatePos(TIME_PER_UPDATE, nullptr, true, true);
        }
    }

}


const std::unordered_set<int> canTurnRight = {0, 3, 5, 8};
const std::unordered_set<int> canTurnLeft = { 4, 9 };
const std::unordered_set<int> mustTurnLeft = { 2, 7 };
const std::unordered_set<int> xPlaneLanes = { 0, 1, 2, 5, 6, 7 };
const std::unordered_set<int> yPlaneLanes = {3, 4, 8, 9};

void newCars(std::vector<std::vector<Car>>& lanes, std::vector<std::vector<Car>>& lanesRead) {
    std::unordered_set<int> alreadyPicked;

    for (int i = 0; i < CARS_PER_INTERVAL; i++) {
        int lane = randLane();

        if (xPlaneLanes.contains(lane) && lanes[lane].size() >= 17) {
            alreadyPicked.insert(lane); //skip if the lane is full
        }
        else if (yPlaneLanes.contains(lane) && lanes[lane].size() >= 15) {
            alreadyPicked.insert(lane); //skip if the lane is full
        }

        if (alreadyPicked.size() == 10) {
            return; //all lanes full
        }

        while (alreadyPicked.contains(lane)) {
            lane = randLane();
        }

        alreadyPicked.insert(lane);
        char turnDir = 's';

        if ((canTurnLeft.contains(lane) && rand() % 2 == 0) || mustTurnLeft.contains(lane)) {
            turnDir = 'l';
        }
        else if (canTurnRight.contains(lane) && rand() % 2 == 0) {
            turnDir = 'r';
        } 

        Car car(lane, turnDir, CAR_MAX_SPEED, CAR_ACCELERATION); 
        lanes[lane].push_back(car);
        lanesRead = lanes;
    }
}


int main()
{
    sf::RenderWindow window(sf::VideoMode{ sf::Vector2u{ WINDOW_WIDTH, WINDOW_HEIGHT } }, sf::String{ "4-Way Intersection" });
    window.setFramerateLimit(60);

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("background2.png")) {
        std::cerr << "Error: Could not load background.png" << std::endl;
        return -1;
    }

    std::vector<std::vector<Car>> lanes(10);
    std::vector<std::vector<Car>> lanesRead(10);
    std::vector<std::vector<Car>> lanesPastLight(10);
    TrafficLight trafficLight(lanesRead);
    std::thread trafficThread(&TrafficLight::run, &trafficLight);

    sf::Sprite backgroundSprite(backgroundTexture);
    sf::Clock clock;
    float accumulatedTimeU = 0.0f;
    float accumulatedTimeN = 0.0f;
    float resTime;

    while (window.isOpen()) {
        resTime = clock.restart().asSeconds();
        accumulatedTimeU += resTime;
        accumulatedTimeN += resTime;

        while (const std::optional event = window.pollEvent()) {
        
            if (event->is<sf::Event::Closed>()) {
            
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();
            }
        }

        window.draw(backgroundSprite);

        if (accumulatedTimeN >= NEW_CAR_INTERVAL) {
            newCars(lanes, lanesRead);
            accumulatedTimeN = 0.0f;
        }

        while (accumulatedTimeU >= TIME_PER_UPDATE) {
            updateLanes(trafficLight, lanes, lanesRead, lanesPastLight);
            accumulatedTimeU -= TIME_PER_UPDATE;
        }

        drawCars(window, lanes, lanesPastLight);
        window.display();
    }
}