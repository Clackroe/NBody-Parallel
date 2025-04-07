#ifndef VECTOR_HPP
#define VECTOR_HPP

struct Vec2 {
    double x = 0, y = 0;
};

Vec2 sub(Vec2 a, Vec2 b);
Vec2 add(Vec2 a, Vec2 b);
Vec2 scale(Vec2 a, double s);

double distSqrd(Vec2 v1, Vec2 v2);

double dist(Vec2 v1, Vec2 v2);

Vec2 Direction(Vec2 from, Vec2 to);

#endif // VECTOR_HPP
