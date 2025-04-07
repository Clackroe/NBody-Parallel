

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Body.hpp"
#include <vector>
void WriteGif(const std::vector<std::vector<unsigned char>>& frames, int width, int height, const char* filename, int delay = 4);

std::vector<unsigned char> RenderFrame(std::vector<Body>& bodies, int width, int height);

#endif // RENDERER_HPP
