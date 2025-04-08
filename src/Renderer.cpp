#include "gif.h"
#include <Renderer.hpp>
#include <cmath>
#include <filesystem>
#include <vector>

void WriteGif(const std::vector<std::vector<unsigned char>>& frames, int width, int height, const char* filename, int delay)
{
    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
    }

    GifWriter g;
    GifBegin(&g, filename, width, height, delay);

    for (const auto& frame : frames) {
        GifWriteFrame(&g, frame.data(), width, height, delay);
    }

    GifEnd(&g);
}

std::vector<unsigned char> RenderFrame(std::vector<Body>& bodies, int width, int height)
{
    std::vector<unsigned char> frame(width * height * 4, 0);

#pragma parallel for
    for (int i = 0; i < bodies.size(); i++) {
        const Body& b = bodies[i];
        int cx = (int)((b.position.x));
        int cy = (int)((b.position.y));

        int radius = (int)(std::sqrt(b.mass));
        radius = std::max(radius, 1);

        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int x = cx + dx;
                int y = cy + dy;
                if (x < 0 || x >= width || y < 0 || y >= height)
                    continue;
                if (dx * dx + dy * dy <= radius * radius) {
                    int idx = (y * width + x) * 4;
                    frame[idx + 0] = 255;
                    frame[idx + 1] = 255;
                    frame[idx + 2] = 255;
                    frame[idx + 3] = 255;
                }
            }
        }
    }
    return std::move(frame);
}
