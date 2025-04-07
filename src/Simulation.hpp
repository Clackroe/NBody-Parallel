#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "Body.hpp"
#include <vector>

void CalculateForcesSequential(std::vector<Body>& bodies, double G);
void UpdateSequential(std::vector<Body>& bodies, double deltaTime, int width, int height);

void CalculateForcesMTBlock(std::vector<Body>& bodies, double G);
void CalculateForcesMTTask(std::vector<Body>& bodies, double G);
void CalculateForcesMTAtomic(std::vector<Body>& bodies, double G);
void CalculateForcesMTCritical(std::vector<Body>& bodies, double G);
void UpdateMT(std::vector<Body>& bodies, double deltaTime, int width, int height);

#endif // SIMULATION_HPP
