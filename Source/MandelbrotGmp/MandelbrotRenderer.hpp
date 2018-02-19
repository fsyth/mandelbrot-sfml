#pragma once

#include <SFML/Graphics.hpp>
#include "View.hpp"

class MandelbrotRenderer
{
public:
    MandelbrotRenderer(int width, int height);
    ~MandelbrotRenderer();
    void run();

private:
    enum class RenderingState
    {
        Rendering,
        Completed,
        Displayed
    };

    int m_width;
    int m_height;
    int m_bufferSizeBytes;
    sf::RenderWindow m_window;
    sf::Thread m_drawingThread;
    sf::Thread m_renderingThread;
    sf::Texture m_buffer;
    sf::Sprite m_sprite;
    sf::Uint8* m_renderingPixels;
    sf::Uint8* m_completedPixels;
    View m_renderingView;
    View m_completedView;
    bool m_cancelling = false;
    bool m_resizing = false;
    RenderingState m_renderingState = RenderingState::Rendering;

    void handleEvents();
    void handleKeys(const sf::Event& event);
    void handleMousePressed(const sf::Event& event);
    void handleMouseMoved(const sf::Event& event);
    void handleMouseReleased(const sf::Event& event);
    void handleMouseWheel(const sf::Event& event);
    void handleResize(const sf::Event& event);

    void draw();
    void render();
    void cancelRendering();
    double mandelbrot(const Complex z0);
    void detailedDraw();
    void roughDraw();


};

sf::Color hueToRGB(double h);
