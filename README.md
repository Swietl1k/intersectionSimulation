# 🚦 Intersection Traffic Simulation

A real-time traffic light simulation built using C++ and SFML, designed to visualize and manage the flow of traffic across a 4-way intersection with multiple lanes and directional signals. The intersection of choice is located in Cracow, on the Armii Krajowej street (50°04'12.2"N 19°54'14.3"E).

---

## 🎮 Description

This application simulates how traffic flows through an intersection with left, straight, and right-turn lanes for all four directions (north, south, east, west). The system visually represents traffic lights and green turn arrows, dynamically updating their states based on a custom traffic signal control system.

---

## 🔧 Key Features

- **4-Way Intersection Visualization**  
  Includes separate lane groupings for all directions, each capable of left, straight, or right movements.
  
- **Traffic Light System**  
  Each lane is governed by its own traffic light represented using colored rectangles (red, yellow, green). 
  
- **Realistic Car Physics**  
    Vehicles exhibit realistic acceleration, deceleration, and turning behaviors, with speed adjustments based on traffic light states and proximity to other vehicles. Cars can brake smoothly, follow other cars at a safe distance, and execute realistic turns based on their lane and desired direction of travel.

- **Custom Signal Controller**  
  Implemented a simplified and more "fair" version of the max pressure traffic control to eliminate the starvation of lanes with the least traffic (can be extended for AI integration).

---

## 🛠️ Technologies Used

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)  
![SFML](https://img.shields.io/badge/SFML-8CC445?style=for-the-badge&logo=sfml&logoColor=white)

---

## 🚀 Future Improvements

- Integrate an AI algorithm for traffic control
- Improve graphics

---
