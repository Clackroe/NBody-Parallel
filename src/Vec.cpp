
#include "Vec.hpp"
#include <cmath>

Vec2 sub(Vec2 a, Vec2 b)
{
    return { a.x - b.x, a.y - b.y };
}

Vec2 add(Vec2 a, Vec2 b)
{
    return { a.x + b.x, a.y + b.y };
}

Vec2 scale(Vec2 a, double s)
{
    return { a.x * s, a.y * s };
}

double distSqrd(Vec2 v1, Vec2 v2)
{
    Vec2 diff = sub(v1, v2);
    double asq = diff.x * diff.x;
    double bsq = diff.y * diff.y;
    return asq + bsq;
}

double dist(Vec2 v1, Vec2 v2)
{
    return std::sqrt(distSqrd(v1, v2));
}

Vec2 Direction(Vec2 from, Vec2 to)
{
    Vec2 dir = sub(to, from);
    double distance = dist(from, to);

    if (distance < 0.000001) {
        return { 0, 0 };
    }
    return { dir.x / distance, dir.y / distance };
}
