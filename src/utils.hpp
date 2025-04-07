#ifndef UTILS_HPP
#define UTILS_HPP

#include <random>
static double generate_random_double(double min_val, double max_val)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min_val, max_val);

    return dis(gen);
}

#endif // UTILS_HPP
