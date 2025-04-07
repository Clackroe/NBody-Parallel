#include "Body.hpp"
#include <Simulation.hpp>
#include <Vec.hpp>
#include <omp.h>

void CalculateForcesSequential(std::vector<Body>& bodies, double G)
{
    for (int i = 0; i < bodies.size(); i++) {
        for (int j = i + 1; j < bodies.size(); j++) {
            if (i == j)
                continue;

            Body& b1 = bodies[i];
            Body& b2 = bodies[j];
            double force = Force(b1, b2, G);

            Vec2 dir = Direction(b1.position, b2.position);
            double acc1 = force / b1.mass; // F = ma => a = F / m
            double acc2 = force / b2.mass; // F = ma => a = F / m

            Vec2 forceVec1 = scale(dir, acc1);
            Vec2 forceVec2 = scale(scale(dir, -1), acc2);

            b1.velocity = add(b1.velocity, forceVec1);
            b2.velocity = add(b2.velocity, forceVec2);
        }
    }
}

void UpdateSequential(std::vector<Body>& bodies, double deltaTime, int width, int height)
{
    for (Body& b : bodies) {
        if (b.position.x < 0 || b.position.x > width) {
            b.velocity.x *= -1;
        }
        if (b.position.y < 0 || b.position.y > height) {
            b.velocity.y *= -1;
        }

        b.position = add(b.position, scale(b.velocity, deltaTime));
    }
}

void CalculateForcesMTBlock(std::vector<Body>& bodies, double G)
{

    const int n = bodies.size();
    std::vector<Vec2> accelerations(n, { 0, 0 });

#pragma omp parallel
    {
        std::vector<Vec2> local_accel(n, { 0, 0 });

#pragma omp for schedule(dynamic)
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                const Body& b1 = bodies[i];
                const Body& b2 = bodies[j];
                double force = Force(b1, b2, G);
                Vec2 dir = Direction(b1.position, b2.position);

                double acc1 = force / b1.mass;

                double acc2 = force / b2.mass;

                Vec2 acc_vec1 = scale(dir, acc1);

                Vec2 acc_vec2 = scale(scale(dir, -1), acc2);

                local_accel[i] = add(local_accel[i], acc_vec1);

                local_accel[j] = add(local_accel[j], acc_vec2);
            }
        }

#pragma omp critical
        {
            for (int i = 0; i < n; i++) {
                accelerations[i] = add(accelerations[i], local_accel[i]);
            }
        }
    }

#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        bodies[i].velocity = add(bodies[i].velocity, accelerations[i]);
    }
}

void CalculateForcesMTTask(std::vector<Body>& bodies, double G)
{

    std::vector<Vec2> accelerations(bodies.size(), { 0, 0 });

#pragma omp parallel
    {
#pragma omp single
        {
            for (int i = 0; i < bodies.size(); i++) {
                for (int j = i + 1; j < bodies.size(); j++) {
#pragma omp task firstprivate(i, j)
                    {
                        const Body& b1 = bodies[i];
                        const Body& b2 = bodies[j];
                        double force = Force(b1, b2, G);
                        Vec2 dir = Direction(b1.position, b2.position);

                        double acc1 = force / b1.mass;
                        double acc2 = force / b2.mass;

                        Vec2 forceVec1 = scale(dir, acc1);
                        Vec2 forceVec2 = scale(scale(dir, -1), acc2);

#pragma omp atomic
                        accelerations[i].x += forceVec1.x;

#pragma omp atomic
                        accelerations[i].y += forceVec1.y;

#pragma omp atomic
                        accelerations[j].x += forceVec2.x;
#pragma omp atomic
                        accelerations[j].y += forceVec2.y;
                    }
                }
            }
        }
    }

#pragma omp parallel for
    for (int i = 0; i < accelerations.size(); i++) {
        bodies[i].velocity = add(bodies[i].velocity, accelerations[i]);
    }
}

void CalculateForcesMTAtomic(std::vector<Body>& bodies, double G)
{
    std::vector<Vec2> accelerations(bodies.size(), { 0, 0 });

#pragma omp parallel for
    for (int i = 0; i < bodies.size(); i++) {
        for (int j = i + 1; j < bodies.size(); j++) {
            if (i == j)
                continue;

            const Body& b1 = bodies[i];
            const Body& b2 = bodies[j];
            double force = Force(b1, b2, G);
            Vec2 dir = Direction(b1.position, b2.position);

            double acc1 = force / b1.mass; // F = ma => a = F / m
            double acc2 = force / b2.mass; // F = ma => a = F / m
            Vec2 forceVec1 = scale(dir, acc1);
            Vec2 forceVec2 = scale(scale(dir, -1), acc2);

#pragma omp atomic
            accelerations[i].x += forceVec1.x;

#pragma omp atomic
            accelerations[i].y += forceVec1.y;

#pragma omp atomic
            accelerations[j].x += forceVec2.x;
#pragma omp atomic
            accelerations[j].y += forceVec2.y;
        }
    }

#pragma omp parallel for
    for (int i = 0; i < accelerations.size(); i++) {
        bodies[i].velocity = add(bodies[i].velocity, accelerations[i]);
    }
}

void CalculateForcesMTCritical(std::vector<Body>& bodies, double G)
{

    std::vector<Vec2> accelerations(bodies.size(), { 0, 0 });

#pragma omp parallel for
    for (int i = 0; i < bodies.size(); i++) {
        for (int j = i + 1; j < bodies.size(); j++) {
            if (i == j)
                continue;

            const Body& b1 = bodies[i];
            const Body& b2 = bodies[j];
            double force = Force(b1, b2, G);
            Vec2 dir = Direction(b1.position, b2.position);

            double acc1 = force / b1.mass; // F = ma => a = F / m
            double acc2 = force / b2.mass; // F = ma => a = F / m
            Vec2 forceVec1 = scale(dir, acc1);
            Vec2 forceVec2 = scale(scale(dir, -1), acc2);

#pragma omp critical
            {
                accelerations[i] = add(accelerations[i], forceVec1);
                accelerations[j] = add(accelerations[j], forceVec2);
            }
        }
    }

#pragma omp parallel for
    for (int i = 0; i < accelerations.size(); i++) {
        bodies[i].velocity = add(bodies[i].velocity, accelerations[i]);
    }
}
void UpdateMT(std::vector<Body>& bodies, double deltaTime, int width, int height)
{

#pragma omp parallel for
    for (int i = 0; i < bodies.size(); i++) {
        Body& b = bodies[i];
        if (b.position.x < 0 || b.position.x > width) {
            b.velocity.x *= -1;
        }
        if (b.position.y < 0 || b.position.y > height) {
            b.velocity.y *= -1;
        }

        b.position = add(b.position, scale(b.velocity, deltaTime));
    }
}
