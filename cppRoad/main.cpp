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
const int NEW_CAR_INTERVAL = 2;
const int CARS_PER_INTERVAL = 5; // cant be more than 10
const int CAR_MAX_SPEED = 60;
const int CAR_ACCELERATION = 20;
const int UPDATE_STATS_INTERVAL = 5;
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


void updateLanes(TrafficLight& trafficLight, std::vector<std::vector<Car>>& lanes, std::vector<std::vector<Car>>& lanesRead, std::vector<std::vector<Car>>& lanesPastLight, int& carsPassedCount, std::unordered_map<std::string, LightColor> laneStates) {

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
            carsPassedCount++;
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

        int attempts = 0;
        while (true) {
            if (!alreadyPicked.contains(lane)) {
                bool isX = xPlaneLanes.contains(lane);
                bool isY = yPlaneLanes.contains(lane);
                int maxCars = isX ? 17 : (isY ? 15 : 0);

                if (lanes[lane].size() < maxCars) {
                    break;  // found a good lane 
                }

                alreadyPicked.insert(lane);  // add full lane to alreadyPicked
            }

            lane = randLane();

            if ( alreadyPicked.size() >= 10)
                return;  //prevent infinite loops 
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


void drawTrafficLights(sf::RenderWindow& window, const std::unordered_map<std::string, LightColor>& laneStates, sf::Sprite& arrowSprite) {
    for (const auto& [laneKey, color] : laneStates) {
        int lane = laneKey[0] - '0';  // convert first char to int
        char dir = laneKey[1];        // 'l', 's', or 'r'

        float x = 0, y = 0, rotation = 0;
        if (dir == 'r' && color == LightColor::GREEN) {
            if (lane == 0) { x = 500; y = 283; rotation = 90.0f; }
            else if (lane == 3) { x = 483; y = 530; rotation = 180.0f; }
            else if (lane == 5) { x = 280; y = 513; rotation = 270.0f; }
            else if (lane == 8) { x = 303; y = 250; rotation = 0.0f; }

            arrowSprite.setOrigin(sf::Vector2f(13, 13));
            arrowSprite.setPosition(sf::Vector2f(x, y));
            arrowSprite.setRotation(sf::degrees(rotation));
            window.draw(arrowSprite);
            continue; // skip to next light
        }
        else if (dir == 'r' && color != LightColor::GREEN) {
            continue; // skip unnecessary code (we only draw green arrows)
        }

        sf::RectangleShape light;
        // set color
        switch (color) {
        case LightColor::GREEN:  
            light.setFillColor(sf::Color::Green); 
            break;
        case LightColor::YELLOW: 
            light.setFillColor(sf::Color::Yellow); 
            break;
        case LightColor::RED:    
            light.setFillColor(sf::Color::Red); 
            break;
        }

        // position the rectangle near the end of the lane
        if (eastLanes.contains(lane)) {
            x = 480;
            y = 310 + lane * 30;
            light.setSize(sf::Vector2f(10.f, 30.f));
        }
        else if (westLanes.contains(lane)) {
            x = 310;
            y = 460 - (lane - 5) * 30;
            light.setSize(sf::Vector2f(10.f, 30.f));
        }
        else if (northLanes.contains(lane)) {
            x = 340 + (lane - 8) * 30;
            y = 280;
            light.setSize(sf::Vector2f(30.f, 10.f));
        }
        else if (southLanes.contains(lane)) {
            x = 430 - (lane - 3) * 30;
            y = 510;
            light.setSize(sf::Vector2f(30.f, 10.f));
        }

        light.setPosition(sf::Vector2f(x, y));
        window.draw(light);
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode{ sf::Vector2u{ WINDOW_WIDTH, WINDOW_HEIGHT } }, sf::String{ "4-Way Intersection" });
    window.setFramerateLimit(60);

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("images/background2.png")) {
        std::cerr << "Error: Could not load background.png" << std::endl;
        return -1;
    }
    sf::Texture arrowTexture;
    if (!arrowTexture.loadFromFile("images/arrow.png")) {
        std::cerr << "Error: Could not load arrow.png" << std::endl;
        return -1;
    }
    sf::Sprite arrowSprite(arrowTexture);

    std::vector<std::vector<Car>> lanes(10);
    std::vector<std::vector<Car>> lanesRead(10);
    std::vector<std::vector<Car>> lanesPastLight(10);
    int carsPassedCount = 0;
    TrafficLight trafficLight(lanesRead);
    std::thread trafficThread(&TrafficLight::run, &trafficLight);

    sf::Sprite backgroundSprite(backgroundTexture);
    sf::Clock tempClock;
    sf::Clock clock;
    float accumulatedTimeUpdate = 0.0f;
    float accumulatedTimeNew = 0.0f;
    float accumulatedTimeStat = 0.0f;
    float resTime;

    while (window.isOpen()) {
        resTime = tempClock.restart().asSeconds();
        accumulatedTimeUpdate += resTime;
        accumulatedTimeNew += resTime;
        accumulatedTimeStat += resTime;

        while (const std::optional event = window.pollEvent()) {
        
            if (event->is<sf::Event::Closed>()) {
            
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();
            }
        }

        window.clear();
        window.draw(backgroundSprite);

        if (accumulatedTimeNew >= NEW_CAR_INTERVAL) {
            newCars(lanes, lanesRead);
            accumulatedTimeNew = 0.0f;
        }

        while (accumulatedTimeUpdate >= TIME_PER_UPDATE) {
            std::unordered_map<std::string, LightColor> laneStates = trafficLight.getAllStates();
            updateLanes(trafficLight, lanes, lanesRead, lanesPastLight, carsPassedCount, laneStates);
            drawTrafficLights(window, laneStates, arrowSprite);
            accumulatedTimeUpdate -= TIME_PER_UPDATE;
        }

        if (accumulatedTimeStat >= UPDATE_STATS_INTERVAL) {
            std::cout << (carsPassedCount * 60) / clock.getElapsedTime().asSeconds() << " cars per minute" << std::endl;
            accumulatedTimeStat = 0;
        }

        drawCars(window, lanes, lanesPastLight);
        window.display();
    }
}