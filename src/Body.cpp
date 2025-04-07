#include <Body.hpp>

double Force(const Body& b1, const Body& b2, double G)
{
    static const double eps = 0.1;
    double distsq = distSqrd(b1.position, b2.position) + eps;

    return b1.mass * b2.mass * G / distsq;
};
