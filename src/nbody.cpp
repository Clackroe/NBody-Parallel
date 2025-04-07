#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <omp.h>

#include "Vec.hpp"
#include <Simulation.hpp>

#include "utils.hpp"
#include <Body.hpp>
#include <Renderer.hpp>

#include <ostream>
#include <vector>

#define G 9.8
#define WIDTH 1920
#define HEIGHT 1080

const double DT = 0.07;

std::vector<Body>
CopyBodies(const std::vector<Body>& bodies)
{
    std::vector<Body> out(bodies.size());
    std::vector<Body> bodiesRender(bodies.size());
    for (int i = 0; i < out.size(); i++) {
        out[i] = bodies[i];
    }
    return out;
}

double BenchMark(std::function<void(std::vector<Body>&, double)> calc, std::function<void(std::vector<Body>&, double, int, int)> update, std::vector<Body>& bodies, const std::string& name, int frames)
{
    const auto start(std::chrono::steady_clock::now());
    for (int i = 0; i < frames; i++) {
        calc(bodies, G);
        update(bodies, DT, WIDTH, HEIGHT);
    }
    const auto end(std::chrono::steady_clock::now());
    std::chrono::duration<double> seconds { end - start };

    std::cout << name << " took: " << seconds.count() << " Seconds" << std::endl;

    return seconds.count();
}

Vec2 CompareFinalPositions(const std::vector<Body>& sequential, const std::vector<Body>& bodies)
{
    Vec2 cumulative = { 0, 0 };

    for (int i = 0; i < bodies.size(); i++) {
        cumulative = add(cumulative, sub(sequential[i].position, bodies[i].position));
    }

    return { cumulative.x / bodies.size(), cumulative.y / bodies.size() };
}

void BenchMarkAllCSV(const std::vector<Body>& originalBodies, const std::string& csvPath, const std::vector<int>& bodyCounts)
{
    if (std::filesystem::exists(csvPath)) {
        std::filesystem::remove(csvPath);
    }

    printf("======== NUM BODIES SCALING BENCHMARK ========\n");

    std::fstream stream(csvPath, std::ios::out);
    stream << "Method,NumBodies,Seconds\n";

    int fixedFrames = 1;
    for (int numBodies : bodyCounts) {
        printf("--- BODIES: %d ---\n\n", numBodies);

        std::vector<Body> bodies(originalBodies.begin(), originalBodies.begin() + numBodies);

        std::vector<Body> seqBds = CopyBodies(bodies);
        std::string name = "Sequential";
        double time = BenchMark(CalculateForcesSequential, UpdateSequential, seqBds, name, fixedFrames);
        stream << name << "," << numBodies << "," << fixedFrames << "," << time << "\n";

        {
            std::vector<Body> bds = CopyBodies(bodies);
            name = "MultiThreaded (Block)";
            time = BenchMark(CalculateForcesMTBlock, UpdateMT, bds, name, fixedFrames);
            Vec2 avgDiff = CompareFinalPositions(seqBds, bds);
            stream << name << "," << numBodies << "," << fixedFrames << "," << time << "\n";
        }

        {
            std::vector<Body> bds = CopyBodies(bodies);
            name = "MultiThreaded (Atomic)";
            time = BenchMark(CalculateForcesMTAtomic, UpdateMT, bds, name, fixedFrames);
            Vec2 avgDiff = CompareFinalPositions(seqBds, bds);
            stream << name << "," << numBodies << "," << fixedFrames << "," << time << "\n";
        }

        {
            std::vector<Body> bds = CopyBodies(bodies);
            name = "MultiThreaded (Critical)";
            time = BenchMark(CalculateForcesMTCritical, UpdateMT, bds, name, fixedFrames);
            Vec2 avgDiff = CompareFinalPositions(seqBds, bds);
            stream << name << "," << numBodies << "," << fixedFrames << "," << time << "\n";
        }

        {
            std::vector<Body> bds = CopyBodies(bodies);
            name = "MultiThreaded (Task)";
            time = BenchMark(CalculateForcesMTTask, UpdateMT, bds, name, fixedFrames);
            Vec2 avgDiff = CompareFinalPositions(seqBds, bds);
            stream << name << "," << numBodies << "," << fixedFrames << "," << time << "\n";
        }
    }

    stream.close();
}

#define N 30
#define FRAMES 1000

int main()
{
    std::vector<Body> bodies;
    for (int i = 0; i < N; i++) {
        Body b;
        b.position = { generate_random_double(100, WIDTH - 100), generate_random_double(100, HEIGHT - 100) };
        b.velocity = { 0, 0 };
        b.mass = generate_random_double(10, 100);
        bodies.push_back(b);
    }

    std::vector<int> bodyCounts = { 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000 };
    BenchMarkAllCSV(bodies, "benchmark_results.csv", bodyCounts);

    {
        std::vector<std::vector<unsigned char>> frames;
        std::vector<Body> bds = CopyBodies(bodies);
        for (int i = 0; i < FRAMES; i++) {

            CalculateForcesMTAtomic(bds, G);
            UpdateMT(bds, DT, WIDTH, HEIGHT);

            frames.push_back(std::move(RenderFrame(bds, WIDTH, HEIGHT)));
        }
        WriteGif(frames, WIDTH, HEIGHT, "simulation.gif");
    }

    return 0;
}
