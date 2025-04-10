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
#include <string>
#include <unordered_map>
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

std::vector<Body> GenerateBodiesMT(int size)
{
    std::vector<Body> bodies(size);
#pragma omp parallel for
    for (int i = 0; i < bodies.size(); i++) {
        Body b;
        b.position = { generate_random_double(100, WIDTH - 100), generate_random_double(100, HEIGHT - 100) };
        b.velocity = { 0, 0 };
        b.mass = generate_random_double(10, 100);
        bodies[i] = b;
    }
    return bodies;
}

void BenchMarkAllCSV(const std::string& csvPath, const std::vector<int>& bodyCounts, const std::vector<int>& threadCounts)
{
    if (std::filesystem::exists(csvPath)) {
        std::filesystem::remove(csvPath);
    }

    printf("======== NUM BODIES SCALING BENCHMARK ========\n");

    int fixedFrames = 1;
    typedef std::unordered_map<std::string, std::vector<double>> TimeMap;

    std::vector<TimeMap> times(threadCounts.size());

    for (TimeMap& m : times) {
        for (auto& [name, bTimes] : m) {
            bTimes.resize(bodyCounts.size());
        }
    }

    for (int index = 0; index < bodyCounts.size(); index++) {
        int numBodies = bodyCounts[index];
        auto bodies = GenerateBodiesMT(numBodies);

        printf("--- BODIES: %d ---\n\n", numBodies);

        // Sequential benchmark

        for (int i = 0; i < threadCounts.size(); i++) {
            int threadCount = threadCounts[i];
            omp_set_num_threads(threadCount); // Set the number of threads for each run

            std::vector<Body> seqBds = CopyBodies(bodies);
            std::string name = "Sequential Threads: " + std::to_string(threadCount);
            double time = BenchMark(CalculateForcesSequential, UpdateSequential, seqBds, name, fixedFrames);
            times[i][name].push_back(time);
            {
                std::vector<Body> bds = CopyBodies(bodies);
                name = "MultiThreaded (Reduction - Dynamic) Threads: " + std::to_string(threadCount);
                time = BenchMark(CalculateForcesMTReduction, UpdateMT, bds, name, fixedFrames);
                Vec2 avgDiff = CompareFinalPositions(seqBds, bds);
                times[i][name].push_back(time);
            }

            {
                std::vector<Body> bds = CopyBodies(bodies);
                name = "MultiThreaded (Reduction - Static) Threads: " + std::to_string(threadCount);
                time = BenchMark(CalculateForcesMTReductionStatic, UpdateMT, bds, name, fixedFrames);
                Vec2 avgDiff = CompareFinalPositions(seqBds, bds);
                times[i][name].push_back(time);
            }

            {
                std::vector<Body> bds = CopyBodies(bodies);
                name = "MultiThreaded (Atomic - Dynamic) Threads: " + std::to_string(threadCount);
                time = BenchMark(CalculateForcesMTAtomic, UpdateMT, bds, name, fixedFrames);
                Vec2 avgDiff = CompareFinalPositions(seqBds, bds);
                times[i][name].push_back(time);
            }

            {
                std::vector<Body> bds = CopyBodies(bodies);
                name = "MultiThreaded (Atomic - Static) Threads: " + std::to_string(threadCount);
                time = BenchMark(CalculateForcesMTAtomicStatic, UpdateMT, bds, name, fixedFrames);
                Vec2 avgDiff = CompareFinalPositions(seqBds, bds);
                times[i][name].push_back(time);
            }

            {
                std::vector<Body> bds = CopyBodies(bodies);
                name = "MultiThreaded (Critical) Threads: " + std::to_string(threadCount);
                time = BenchMark(CalculateForcesMTCritical, UpdateMT, bds, name, fixedFrames);
                Vec2 avgDiff = CompareFinalPositions(seqBds, bds);
                times[i][name].push_back(time);
            }
        }
    }

    int index = 0;
    for (auto& m : times) {
        std::fstream stream(std::to_string(index) + "_" + csvPath, std::ios::out);
        index++;
        stream << "Method";
        for (auto num : bodyCounts) {
            stream << "," << num;
        }
        stream << "\n";

        for (auto [name, bTimes] : m) {
            stream << name;
            for (double t : bTimes) {
                stream << "," << t;
            }
            stream << "\n";
        }
        stream.close();
    }
}

#define FRAMES 700
int main()
{
    int num_threads = omp_get_max_threads();
    std::cout << "Default number of threads: " << num_threads << std::endl;
    std::vector<int> bodyCounts = { 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 5000 };

    // Of course, this many threads is alot and would only really be useful on high perf computers
    // (Like mayeb Centaurous)
    std::vector<int> threadCounts = { 1, 2, 4, 8, 16, 32, 64, 128 };

    // BenchMarkAllCSV("benchmark_results.csv", bodyCounts, threadCounts);

    {
        std::vector<Body> bds = GenerateBodiesMT(35);

        std::vector<std::vector<Body>> frameStates;

        for (int i = 0; i < FRAMES; i++) {

            CalculateForcesMTReduction(bds, G);
            UpdateMT(bds, DT, WIDTH, HEIGHT);
            frameStates.push_back(CopyBodies(bds));
        }

        std::cout << "===Rendering....===" << std::endl;

        std::vector<std::vector<unsigned char>> frames(frameStates.size());

#pragma omp parallel for
        for (int i = 0; i < frameStates.size(); i++) {
            frames[i] = std::move(RenderFrame(frameStates[i], WIDTH, HEIGHT));
        }

        WriteGif(frames, WIDTH, HEIGHT, "simulation.gif");
    }

    return 0;
}
