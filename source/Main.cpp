#include <SFML/Graphics.hpp>
#include <iostream>
#include <functional>
#include "View.hpp"

// Screen dimensions, width and height in pixels
const unsigned int SCREEN_W = 800u;
const unsigned int SCREEN_H = 600u;


// Global View object, since it's used in all of the event handler and 
// rendering functions
View view(-0.5, 0.0, 1.0, SCREEN_W, SCREEN_H);


/// <summary>
/// Converts a value for hue at max saturation and brightness to RGB format
/// </summary>
/// <param name="h">Hue value for the colour, range 0:360</param>
/// <returns>Colour in RGB format</returns>
sf::Color hueToRGB(double h) {
    sf::Uint8 x = (sf::Uint8)(0xFF * (1 - abs(fmod((h / 60.0), 2) - 1.0)));
    if      (h <  60) return sf::Color(0xFFu, x, 0u);
    else if (h < 120) return sf::Color(x, 0xFFu, 0u);
    else if (h < 180) return sf::Color(0u, 0xFFu, x);
    else if (h < 240) return sf::Color(0u, x, 0xFFu);
    else if (h < 300) return sf::Color(x, 0u, 0xFFu);
    else if (h < 360) return sf::Color(0xFFu, 0u, x);
    else              return sf::Color();
}


/// <summary>
/// Handles key pressed events.
/// Pans the view with arrow keys / WASD
/// Resets to initial view with R
/// </summary>
/// <param name="event">Key Pressed Event Union</param>
void handleKeys(sf::Event &event)
{
    const Real MOVEMENT_AMOUNT = 0.25;

    switch (event.key.code)
    {
    case sf::Keyboard::Left:
    case sf::Keyboard::A:
        view.moveBy(-MOVEMENT_AMOUNT, 0);
        break;

    case sf::Keyboard::Right:
    case sf::Keyboard::D:
        view.moveBy(MOVEMENT_AMOUNT, 0);
        break;

    case sf::Keyboard::Up:
    case sf::Keyboard::W:
        view.moveBy(0, -MOVEMENT_AMOUNT);
        break;

    case sf::Keyboard::Down:
    case sf::Keyboard::S:
        view.moveBy(0, MOVEMENT_AMOUNT);
        break;

    case sf::Keyboard::R:
        view.moveTo(-0.5, 0);
        view.zoomTo(1);
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
void handleMousePressed(sf::Event &event)
{
    switch (event.mouseButton.button)
    {
    case sf::Mouse::Left:
        view.zoomBoxBegin(event.mouseButton.x, event.mouseButton.y);
        break;

    case sf::Mouse::Right:
        view.zoomBoxCancel();
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
void handleMouseMoved(sf::Event &event)
{
    view.zoomBoxContinue(event.mouseMove.x,
                         event.mouseMove.y);
}

/// <summary>
/// For left click, finish the box zoom, which will apply changes to the view.
/// </summary>
/// <param name="event">Mouse Released Event Union</param>
void handleMouseReleased(sf::Event &event)
{
    if (event.mouseButton.button == sf::Mouse::Left)
        view.zoomBoxEnd(event.mouseButton.x, event.mouseButton.y);
}


/// <summary>
/// Zooms the view in and out with the mouse wheel.
/// </summary>
/// <param name="event"></param>
void handleMouseWheel(sf::Event &event)
{
    const Real ZOOM_AMOUNT = -0.25;
    view.zoomBy(event.mouseWheelScroll.delta * ZOOM_AMOUNT);
}


/// <summary>
/// Polls events from the window and directs them to the appropriate handlers
/// </summary>
/// <param name="window">The window to poll events from</param>
void handleEvents(sf::Window &window)
{
    static sf::Event event;
    while (window.pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            window.close();
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

        default:
            break;
        }
    }
}


/// <summary>
/// Iterates a complex number z using the Mandelbrot Set function
/// z[n+1] := z[n]^2 + z[0]
/// and returns the number of iterations until the number becomes unbounded,
/// divided by the maximum number of iterations.
/// </summary>
/// <param name="z0">The complex number to evaluate</param>
/// <returns>Ratio of iterations until unbounded</returns>
Real mandelbrot(const Complex z0)
{
    const int MAX_ITERATIONS = 120 - (10 * (int)view.getZoom());
    const Real THRESHOLD = 16.0;

    // Initialise the iterated complex number at z0
    Complex z(z0);

    // Track the number of iterations until z becomes unbounded
    for (int n = 0; n < MAX_ITERATIONS; n++)
    {
        // The Mandelbrot set is given by iteration of:
        // z[n+1] := z[n]^2 + z[0]
        // Handling the real and imaginary parts separately:
        z = Complex(z.x * z.x - z.y * z.y + z0.x, 
                    2 * z.x * z.y + z0.y);

        // z is approximately unbounded if its magnitude exceeds some threshold
        if (z.x * z.x + z.y * z.y > THRESHOLD)
            return (Real)n / MAX_ITERATIONS;
    }

    return 1.0;
}


/// <summary>
/// Renders to the supplied pixel array.
/// Optionally, a rectangle can be provided to render only a subset of pixels
/// in the array. This allows multiple threads to render sections of the 
/// overall image.
/// Optionally, an integer pointer can be passed in, which will be incremented
/// once the render is complete. This is useful for tracking the overall 
/// progress of multi-threaded rendering.
/// </summary>
/// <param name="pixels">The pixel array to render to</param>
/// <param name="rect">A rectangle sub-set of pixels to render. If omitted,
/// the entire pixel array will be rendered.</param>
/// <param name="completedPtr">An integer pointer incremented on completion
/// to allow multiple threads to be easily tracked</param>
void renderPixels(
    sf::Uint8 *pixels,
    const sf::Rect<int> &rect = sf::Rect<int>(0, 0, SCREEN_W, SCREEN_H),
    int *completedPtr = NULL)
{
    // Memory offsets beween adjacent pixels
    const int toNextCol = 4;
    const int toNextRow = toNextCol * (SCREEN_W - rect.width);
    sf::Uint8 *currentPixel = pixels + toNextCol * (rect.top * SCREEN_W + rect.left);
    
    // Colour every pixel based on the Mandelbrot set
    for (int y = rect.top; y < rect.top + rect.height; y++, currentPixel += toNextRow)
    {
        for (int x = rect.left; x < rect.left + rect.width; x++, currentPixel += toNextCol)
        {
            // Convert screen pixel coordinate (x,y) to a complex number z
            // in the view (x + yi)
            Complex z = view.complexAtPixel(x, y);

            // Apply the Mandelbrot set to the complex number and get the 
            // proportion of iterations to the maximum until divergence. 
            // If m == 1, the number remained bounded, i.e. in the set.
            Real m = mandelbrot(z);

            // Colour each pixel in the view based on the number of 
            // iterations to unbounded
            // Default black colour if reached max iterations
            sf::Color c;
            if (m < 1)
                c = hueToRGB(360 * m);

            currentPixel[0] = c.r;
            currentPixel[1] = c.g;
            currentPixel[2] = c.b;
            currentPixel[3] = 0xFFu;
        }
    }

    if (completedPtr != NULL)
        (*completedPtr)++;
}


/// /// <summary>
/// Draw the pixels in the buffer of the sprite to the window
/// </summary>
/// <param name="pixels"></param>
/// <param name="buffer"></param>
/// <param name="sprite"></param>
/// <param name="window"></param>
void detailedDraw(
    sf::Uint8 *pixels, 
    sf::Texture &buffer, 
    sf::Sprite &sprite,
    sf::RenderWindow *window)
{
    buffer.update(pixels);
    sprite.setPosition(0, 0);
    sprite.setScale(1, 1);
    window->draw(sprite);
}


/// <summary>
/// Draw the pixels from an old view offset and scaled in the current view.
/// </summary>
/// <param name="goodPixels"></param>
/// <param name="buffer"></param>
/// <param name="sprite"></param>
/// <param name="window"></param>
/// <param name="displayedView"></param>
void roughDraw(
    sf::Uint8 *goodPixels, 
    sf::Texture &buffer, 
    sf::Sprite &sprite,
    sf::RenderWindow *window,
    View &displayedView)
{
    Pixel roughPosition = view.pixelAtComplex(
        displayedView.getViewport().left,
        displayedView.getViewport().top);
    Real roughScale = displayedView.getScale() / view.getScale();
    buffer.update(goodPixels);
    sprite.setScale(roughScale, roughScale);
    sprite.setPosition(roughPosition.x, roughPosition.y);
    
    window->clear();
    window->draw(sprite);
}


/// <summary>
/// Drawing Thread Entry Point
/// Creates a pixel array and buffer to be rendered to.
/// Creates several rendering threads to update the pixel array.
/// Then in the drawing loop, terminates and launches rendering threads
/// whenever the view is dirty and displays the pixel array in the window.
/// </summary>
/// <param name="window">The window to draw to</param>
void draw(sf::RenderWindow *window)
{
    // Enable an OpenGL context from this thread
    window->setActive(true);

    // Create a buffer the same size as the window and a sprite to draw it.
    // It is faster to draw to an array of pixels and update the buffer later
    // than to draw to the buffer directly on the GPU.
    // The pixel array is created on the heap, otherwise stack overflow occurs.
    const sf::Vector2u SCREEN_SIZE = window->getSize();
    const int BUFFER_SIZE_BYTES = 4 * SCREEN_SIZE.x * SCREEN_SIZE.y;
    sf::Texture buffer;
    sf::Uint8 *pixels = new sf::Uint8[BUFFER_SIZE_BYTES];
    sf::Uint8 *goodPixels = new sf::Uint8[BUFFER_SIZE_BYTES];
    sf::Sprite sprite;
    buffer.create(SCREEN_SIZE.x, SCREEN_SIZE.y);
    sprite.setTexture(buffer);
    View displayedView = view;

    // Parameters for multi-threaded software rendering.
    // Each rendering thread will render a sub-rectangle of the pixel array.
    // The rendering threads are stored in an array of Thread pointers since
    // no empty constructor exists for sf::Thread and each thread needs to be
    // initialised with a different rectangle parameter.
    // Each thread is also passed a pointer to an integer, increment in value
    // on completion.
    const bool RENDER_CONTINUOUSLY = true;
    const int PIXEL_DIVISION = 4;
    const int SUB_RENDER_W = SCREEN_SIZE.x / PIXEL_DIVISION;
    const int SUB_RENDER_H = SCREEN_SIZE.y / PIXEL_DIVISION;
    const int THREAD_POOL_SIZE = PIXEL_DIVISION * PIXEL_DIVISION;
    sf::Thread *renderingThreads[THREAD_POOL_SIZE];
    int completedThreads = 0;
    const int ALREADY_DISPLAYED = -1;
    
    for (int i = 0; i < PIXEL_DIVISION; i++)
    {
        for (int j = 0; j < PIXEL_DIVISION; j++)
        {
            renderingThreads[i * PIXEL_DIVISION + j] = new sf::Thread(std::bind(
                &renderPixels,
                pixels,
                sf::Rect<int>(
                    SUB_RENDER_W * i,
                    SUB_RENDER_H * j,
                    SUB_RENDER_W,
                    SUB_RENDER_H),
                &completedThreads));
        }
    }
    
    // Drawing Loop
    while (window->isOpen())
    {
        // Dirty view means changes have occured.
        // Kill off any currently running rendering threads and relaunch
        // them for the updated view
        if (view.isDirty())
        {
            // Prevent rendering threads being relaunched every loop
            view.isDirty(false);

            // Log the view of each rendering
            std::cout << view << std::endl;

            for (int i = 0; i < THREAD_POOL_SIZE; i++)
                renderingThreads[i]->terminate();
            
            completedThreads = 0;

            // Do a rough draw before relaunching rendering threads
            roughDraw(goodPixels, buffer, sprite, window, displayedView);
            window->display();
            // Repeat the draw because of GL double buffering
            window->clear();
            window->draw(sprite);
            window->display();

            // Make pixels transparent until rendering threads set them
            if (RENDER_CONTINUOUSLY)
            {
                for (int i = 0; i < BUFFER_SIZE_BYTES; i++)
                    pixels[i] = 0;
            }
            
            /*
            // Testing
            Complex a = view.getViewportPosition();
            Pixel b1 = view.pixelAtComplex(a);
            Pixel b2 = view.pixelAtComplex(a.x, a.y);
            Complex c11 = view.complexAtPixel(b1);
            Complex c12 = view.complexAtPixel(b1.x, b1.y);
            Complex c21 = view.complexAtPixel(b2);
            Complex c22 = view.complexAtPixel(b2.x, b2.y);
            printf("a   %+e %+e\n",        a.x,        a.y);
            printf("b1  %+e %+e\n", (Real)b1.x, (Real)b1.y);
            printf("b2  %+e %+e\n", (Real)b2.x, (Real)b2.y);
            printf("c11 %+e %+e\n",      c11.x,      c11.y);
            printf("c12 %+e %+e\n",      c12.x,      c12.y);
            printf("c21 %+e %+e\n",      c21.x,      c21.y);
            printf("c22 %+e %+e\n",      c22.x,      c22.y);
            */

            for (int i = 0; i < THREAD_POOL_SIZE; i++)
                renderingThreads[i]->launch();

        }

        //std::cout << completedThreads << std::endl;

        // Display zoom box, if shown
        if (view.getZoomBoxIsShown())
        {
            // Rough draw before drawing the box on top
            roughDraw(goodPixels, buffer, sprite, window, displayedView);
            window->draw(view.getZoomBoxShape());
            window->display();
           
        }

        // Either:
        //   render once all threads have completed
        // or:
        //   render continuously, unless no changes have occured since
        if (completedThreads == THREAD_POOL_SIZE)
        {
            // Display the fully rendered buffer
            detailedDraw(pixels, buffer, sprite, window);
            window->display();

            // Keep a copy of this completed render for rough drawing when
            // moving the view
            for (int i = 0; i < BUFFER_SIZE_BYTES; i++)
                goodPixels[i] = pixels[i];

            // Store the view for the last completed view, so it can be
            // correctly transformed when rough drawing
            displayedView = view;

            // Prevent the completed buffer from being repeatedly displayed
            completedThreads = ALREADY_DISPLAYED;

            if (!RENDER_CONTINUOUSLY)
                std::cout << "Displayed" << std::endl;
        }
        else if (RENDER_CONTINUOUSLY && completedThreads != ALREADY_DISPLAYED)
        {
            // Display the partially rendered buffer
            detailedDraw(pixels, buffer, sprite, window);
            window->display();
        }
    }

    // Clear up resources
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        renderingThreads[i]->terminate();
        delete renderingThreads[i];
    }
    delete[] pixels;
}


/// <summary>
/// Main Entry Point
/// Creates a Window.
/// Launches the drawing thread.
/// Handles events for the window in a loop.
/// </summary>
/// <returns>0 if no error</returns>
int main()
{
    // Create an SFML window with V-Sync
    sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H),
        "Mandelbrot Set Rendered in SFML");
    window.setVerticalSyncEnabled(true);
    window.setActive(false);

    // Draw to the window in a separate thread
    sf::Thread drawingThread(&draw, &window);
    drawingThread.launch();

    // Main Event Loop
    // Handle queued Keyboard/Mouse/OS inputs
    while (window.isOpen())
    {
        handleEvents(window);
    }

    // Wait for drawing thread to clear up resources
    drawingThread.wait();

    return 0;
}
