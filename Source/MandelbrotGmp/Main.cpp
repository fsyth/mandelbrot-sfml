#include "MandelbrotRenderer.hpp"

int main()
{
    // Screen dimensions, width and height in pixels
    constexpr unsigned int SCREEN_W = 1200u;
    constexpr unsigned int SCREEN_H = 900u;

    MandelbrotRenderer mandelbrot(SCREEN_W, SCREEN_H);
    mandelbrot.run();

    return 0;
}
