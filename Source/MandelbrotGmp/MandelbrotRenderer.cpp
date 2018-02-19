#include "MandelbrotRenderer.hpp"
#include <iostream>
#include <functional>


/// <summary>
/// Constructs a MandelbrotRenderer, which creates a window, listens for
/// user input events, and renders the Mandelbrot Set.
/// </summary>
/// <param name="width">The width of the window to create</param>
/// <param name="height">The height of the window to create</param>
MandelbrotRenderer::MandelbrotRenderer(int width, int height) :
    m_width(width),
    m_height(height),
    m_bufferSizeBytes(4 * width * height),
    m_window(sf::VideoMode(width, height),
             "Mandelbrot Set Rendered in SFML with OpenMP and GMP",
             sf::Style::Titlebar | sf::Style::Close/* | sf::Style::Resize*/),
    m_drawingThread(&MandelbrotRenderer::draw, this),
    m_renderingThread(&MandelbrotRenderer::render, this),
    m_renderingView(-0.5, 0.0, 1.0, width, height)
{
    m_window.setFramerateLimit(60);
    m_window.setActive(false);
}


MandelbrotRenderer::~MandelbrotRenderer()
{
    // Note: m_renderingPixels and m_completedPixels are usually freed at the end
    // of MandelbrotRenderer::draw()
    if (m_renderingPixels != nullptr)
        delete[] m_renderingPixels;

    if (m_completedPixels != nullptr)
        delete[] m_completedPixels;
}


void MandelbrotRenderer::run()
{
    // Draw to the window in a separate thread
    m_drawingThread.launch();

    // Main Event Loop
    // Handle queued Keyboard/Mouse/OS inputs
    while (m_window.isOpen())
    {
        handleEvents();
    }

    // Wait for drawing thread to clear up resources
    m_drawingThread.wait();
}


/// <summary>
/// Polls events from the window and directs them to the appropriate handlers
/// </summary>
void MandelbrotRenderer::handleEvents()
{
    static sf::Event event;
    while (m_window.pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            cancelRendering();
            m_window.close();
            break;

        case sf::Event::KeyPressed:
            handleKeys(event);
            break;

        case sf::Event::MouseButtonPressed:
            handleMousePressed(event);
            break;

        case sf::Event::MouseMoved:
            handleMouseMoved(event);
            break;

        case sf::Event::MouseButtonReleased:
            handleMouseReleased(event);
            break;

        case sf::Event::MouseWheelScrolled:
            handleMouseWheel(event);
            break;

        case::sf::Event::Resized:
            handleResize(event);
            break;

        default:
            break;
        }
    }
}


/// <summary>
/// Handles key pressed events.
/// Pans the view with arrow keys / WASD
/// Resets to initial view with R
/// </summary>
/// <param name="event">Key Pressed Event Union</param>
void MandelbrotRenderer::handleKeys(const sf::Event& event)
{
    static const Real MOVEMENT_AMOUNT = 0.25;

    switch (event.key.code)
    {
    case sf::Keyboard::Left:
    case sf::Keyboard::A:
        m_renderingView.moveBy(-MOVEMENT_AMOUNT, 0);
        break;

    case sf::Keyboard::Right:
    case sf::Keyboard::D:
        m_renderingView.moveBy(MOVEMENT_AMOUNT, 0);
        break;

    case sf::Keyboard::Up:
    case sf::Keyboard::W:
        m_renderingView.moveBy(0, -MOVEMENT_AMOUNT);
        break;

    case sf::Keyboard::Down:
    case sf::Keyboard::S:
        m_renderingView.moveBy(0, MOVEMENT_AMOUNT);
        break;

    case sf::Keyboard::R:
        m_renderingView.moveTo(-0.5, 0);
        m_renderingView.zoomTo(1);
        break;

    default:
        break;
    }
}


/// <summary>
/// For left click, begin a box zoom.
/// For right click, cancel it.
/// </summary>
/// <param name="event">Mouse Pressed Event Union</param>
void MandelbrotRenderer::handleMousePressed(const sf::Event& event)
{
    switch (event.mouseButton.button)
    {
    case sf::Mouse::Left:
        m_renderingView.zoomBoxBegin(event.mouseButton.x, event.mouseButton.y);
        break;

    case sf::Mouse::Right:
        m_renderingView.zoomBoxCancel();
        break;

    default:
        break;
    }
}

/// <summary>
/// When the mouse is moved, continue the box zoom so that it's updated
/// shape can be drawn.
/// </summary>
/// <param name="event">Mouse Moved Event Union</param>
void MandelbrotRenderer::handleMouseMoved(const sf::Event& event)
{
    m_renderingView.zoomBoxContinue(event.mouseMove.x, event.mouseMove.y);
}

/// <summary>
/// For left click, finish the box zoom, which will apply changes to the view.
/// </summary>
/// <param name="event">Mouse Released Event Union</param>
void MandelbrotRenderer::handleMouseReleased(const sf::Event& event)
{
    if (event.mouseButton.button == sf::Mouse::Left)
        m_renderingView.zoomBoxEnd(event.mouseButton.x, event.mouseButton.y);
}


/// <summary>
/// Zooms the view in and out with the mouse wheel.
/// </summary>
/// <param name="event"></param>
void MandelbrotRenderer::handleMouseWheel(const sf::Event& event)
{
    static const double ZOOM_AMOUNT = -1;
    m_renderingView.zoomBy(event.mouseWheelScroll.delta * ZOOM_AMOUNT);
}


/// <summary>
/// Resizes intenal buffers and view when the window is resized.
/// </summary>
/// <param name="event">Window Resized Event Union</param>
void MandelbrotRenderer::handleResize(const sf::Event& event)
{
    m_resizing = true;
    cancelRendering();
    m_drawingThread.wait();

    m_width = event.size.width;
    m_height = event.size.height;
    m_bufferSizeBytes = 4 * m_width * m_height;

    m_renderingView.resizeScreen(event.size.width, event.size.height);
    m_completedView.resizeScreen(event.size.width, event.size.height);

    m_resizing = false;
    m_drawingThread.launch();
}


/// <summary>
/// Drawing Thread Entry Point
/// Creates a pixel array and buffer to be rendered to.
/// Creates several rendering threads to update the pixel array.
/// Then in the drawing loop, terminates and launches rendering threads
/// whenever the view is dirty and displays the pixel array in the window.
/// </summary>
void MandelbrotRenderer::draw()
{
    // Enable an OpenGL context from this thread
    m_window.setActive(true);

    // Create a buffer the same size as the window and a sprite to draw it.
    // It is faster to draw to an array of pixels and update the buffer later
    // than to draw to the buffer directly on the GPU.
    // The pixel array is created on the heap, otherwise stack overflow occurs.
    m_renderingPixels = new sf::Uint8[m_bufferSizeBytes];
    m_completedPixels = new sf::Uint8[m_bufferSizeBytes];
    m_buffer.create(m_width, m_height);
    m_sprite.setTexture(m_buffer);
    m_completedView = m_renderingView;

    // Drawing Loop
    while (m_window.isOpen() && !m_resizing)
    {
        // Dirty view means changes have occured.
        // Kill off any currently running rendering threads and relaunch
        // them for the updated view
        if (m_renderingView.isDirty())
        {
            // Prevent rendering threads being relaunched every loop
            m_renderingView.isDirty(false);

            // Log the view of each rendering
            std::cout << m_renderingView << std::endl;

            // No point running rendering threads for an outdated view
            cancelRendering();

            // Do a rough draw before relaunching rendering threads
            roughDraw();
            m_window.display();

            // Repeat the draw because of GL double buffering
            m_window.clear();
            m_window.draw(m_sprite);
            m_window.display();

            // Make pixels transparent until rendering threads set them
#pragma omp parallel for
            for (int i = 0; i < m_bufferSizeBytes; ++i)
                m_renderingPixels[i] = 0;

            // Begin rendering
            m_renderingState = RenderingState::Rendering;
            m_renderingThread.launch();
        }

        // Can avoid drawing anything if nothing has changed.
        bool shouldDisplay = false;

        // Draw the last completed view if anything will be superimposed on top of it
        // In other words, if the current rendering is incomplete so partially transparent,
        // or if the zoom box will require a redraw of the background.
        if (m_renderingState == RenderingState::Rendering || m_renderingView.getZoomBoxIsShown())
        {
            roughDraw();
            shouldDisplay = true;
        }

        // Draw the rendering buffer, unless it has already been displayed with no changes since.
        if (m_renderingState != RenderingState::Displayed)
        {
            // Draw the partial or complete render
            detailedDraw();
            shouldDisplay = true;
        }

        // If the render has just been completed, copy it to the completed buffer
        if (m_renderingState == RenderingState::Completed)
        {
            // Keep a copy of this completed render for rough drawing when
            // moving the view
#pragma omp parallel for
            for (int i = 0; i < m_bufferSizeBytes; ++i)
                m_completedPixels[i] = m_renderingPixels[i];

            // Store the view for the last completed view, so it can be
            // correctly transformed when rough drawing
            m_completedView = m_renderingView;

            // Prevent the completed buffer from being repeatedly displayed
            m_renderingState = RenderingState::Displayed;
        }

        // Display zoom box on top of everything if it should be shown
        if (m_renderingView.getZoomBoxIsShown())
        {
            m_window.draw(m_renderingView.getZoomBoxShape());
            shouldDisplay = true;
        }

        if (shouldDisplay)
            m_window.display();
    }

    // Free the buffers
    delete[] m_renderingPixels;
    delete[] m_completedPixels;

    m_renderingPixels = nullptr;
    m_completedPixels = nullptr;

    m_window.setActive(false);
}


/// <summary>
/// Renders to the m_renderingPixels buffer.
/// Can be made to return early by setting m_cancelling to true.
/// This function increments m_renderingState when it finishes.
/// </summary>
void MandelbrotRenderer::render()
{
    // Colour every pixel based on the Mandelbrot set
#pragma omp parallel for
    for (int y = 0; y < m_height; ++y)
    {
        // Check early exit flag
        if (m_cancelling)
            break;

        for (int x = 0; x < m_width; ++x)
        {
            // Convert screen pixel coordinate (x,y) to a complex number z
            // in the view (x + yi)
            Complex z = m_renderingView.complexAtPixel(x, y);

            // Apply the Mandelbrot set to the complex number and get the
            // proportion of iterations to the maximum until divergence.
            // If m == 1, the number remained bounded, i.e. in the set.
            double m = mandelbrot(z);

            // Colour each pixel in the view based on the number of
            // iterations to unbounded
            // Default black colour if reached max iterations
            sf::Color c;
            if (m < 1)
                c = hueToRGB(360 * m);

            // Colour in the buffer
            sf::Uint8 *currentPixel = m_renderingPixels + 4 * (y * m_width + x);
            currentPixel[0] = c.r;
            currentPixel[1] = c.g;
            currentPixel[2] = c.b;
            currentPixel[3] = 0xFFu;
        }
    }

    // Rendering now complete
    m_renderingState = RenderingState::Completed;
}


/// <summary>
/// Sets a flag to cause the rendering thread to exit early and waits for it to finish.
/// </summary>
void MandelbrotRenderer::cancelRendering()
{
    m_cancelling = true;
    m_renderingThread.wait();
    m_cancelling = false;
}


/// <summary>
/// Iterates a complex number z using the Mandelbrot Set function
/// z[n+1] := z[n]^2 + z[0]
/// and returns the number of iterations until the number becomes unbounded,
/// divided by the maximum number of iterations.
/// </summary>
/// <param name="z0">The complex number to evaluate</param>
/// <returns>Ratio of iterations until unbounded</returns>
double MandelbrotRenderer::mandelbrot(const Complex z0)
{
    const int MAX_ITERATIONS = 120 - (10 * static_cast<int>(m_renderingView.getZoom()));
    constexpr double THRESHOLD = 16.0;
    static const Real TWO = 2.0;

    // Initialise the iterated complex number at z0
    Complex z(z0);

    // Track the number of iterations until z becomes unbounded
    for (int n = 0; n < MAX_ITERATIONS; n++)
    {
        // The Mandelbrot set is given by iteration of:
        // z[n+1] := z[n]^2 + z[0]
        // Handling the real and imaginary parts separately:
        z = Complex(z.x * z.x - z.y * z.y + z0.x,
                    TWO * z.x * z.y + z0.y);

        // z is approximately unbounded if its magnitude exceeds some threshold
        if (z.x * z.x + z.y * z.y > THRESHOLD)
            return static_cast<double>(n) / static_cast<double>(MAX_ITERATIONS);
    }

    return 1.0;
}


/// <summary>
/// Draw the rendering pixels.
/// If the MandelbrotRenderer::render() has not finished, some pixels
/// in m_renderingPixels will be transparent.
/// </summary>
void MandelbrotRenderer::detailedDraw()
{
    m_buffer.update(m_renderingPixels);
    m_sprite.setPosition(0, 0);
    m_sprite.setScale(1, 1);
    m_window.draw(m_sprite);
}


/// <summary>
/// Draw the pixels from the last completed rendering.
/// The image will be offset and scaled from the completed view to the current view.
/// </summary>
void MandelbrotRenderer::roughDraw()
{
    m_buffer.update(m_completedPixels);

    Pixel roughPosition = m_renderingView.pixelAtComplex(
        m_completedView.getViewport().left,
        m_completedView.getViewport().top);

    m_sprite.setPosition(static_cast<float>(roughPosition.x),
                         static_cast<float>(roughPosition.y));

    Real roughScale = m_completedView.getScale() / m_renderingView.getScale();

    m_sprite.setScale(static_cast<float>(roughScale),
                      static_cast<float>(roughScale));

    m_window.clear();
    m_window.draw(m_sprite);
}


/// <summary>
/// Converts a value for hue at max saturation and brightness to RGB format
/// </summary>
/// <param name="h">Hue value for the colour, range 0:360</param>
/// <returns>Colour in RGB format</returns>
sf::Color hueToRGB(double h) {
    sf::Uint8 x = static_cast<sf::Uint8>(0xFF * (1 - abs(fmod((h / 60.0), 2) - 1.0)));

    switch (static_cast<int>(h) / 60)
    {
    case 0:
        return sf::Color(0xFF, x, 0);
    case 1:
        return sf::Color(x, 0xFF, 0);
    case 2:
        return sf::Color(0, 0xFF, x);
    case 3:
        return sf::Color(0, x, 0xFF);
    case 4:
        return sf::Color(x, 0, 0xFF);
    case 5:
        return sf::Color(0xFF, 0, x);
    default:
        return sf::Color();
    }
}
