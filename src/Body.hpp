#ifndef BODY_HPP
#define BODY_HPP
#include <Vec.hpp>

struct Body {
    Vec2 position;
    Vec2 velocity;
    double mass;
};

double Force(const Body& b1, const Body& b2, double G);

#endif // BODY_HPP
