#include "View.hpp"

/// <summary>
/// Constructor
/// </summary>
/// <param name="x">Centre X coordinate of the view</param>
/// <param name="y">Centre X coordinate of the view</param>
/// <param name="zoom">Zoom level of the view</param>
/// <param name="screenWidth">Width of the screen, only required when 
/// converting pixel coordinates to complex numbers</param>
/// <param name="screenHeight">Height of the screen, only required when 
/// converting pixel coordinates to complex numbers</param>
View::View(Real x, Real y, Real zoom, int screenWidth, int screenHeight)
{
    // Screen size must be set first as aspect ratio ust be known to update
    // viewport
    screenSize = Pixel(screenWidth, screenHeight);
    aspectRatio = (Real)screenWidth / screenHeight;
    moveTo(x, y);
    zoomTo(zoom);

    zoomBoxShape.setFillColor(sf::Color::Transparent);
    zoomBoxShape.setOutlineThickness(1);
    zoomBoxShape.setOutlineColor(sf::Color(0x808080));
}

/// <summary>
/// Destructor
/// </summary>
View::~View() {}


/// <summary>
/// Getter for dirty.
/// If the view is dirty, changes have occured to the view recently indicating
/// that a re-render is required.
/// </summary>
/// <returns>dirty</returns>
bool View::isDirty() { return dirty; }

/// <summary>
/// Setter for dirty.
/// The view should be set to dirty whenever changes are made to it.
/// The view should be set to not dirty when the view is rendered.
/// </summary>
/// <param name="d">The new value for dirty</param>
void View::isDirty(bool d) { dirty = d; }


/// <summary>
/// Getter for scale.
/// </summary>
/// <returns>scale</returns>
Real View::getScale() { return scale; }

/// <summary>
/// Setter for scale.
/// Also updates the values for zoom and viewport accordingly.
/// View is now dirty.
/// </summary>
/// <param name="s">The new value for scale</param>
void View::setScale(Real s)
{
    scale = s;
    zoom = log2(s);
    isDirty(true);
    updateViewport();
}


/// <summary>
/// Getter for zoom.
/// </summary>
/// <returns>zoom</returns>
Real View::getZoom() { return zoom; }

/// <summary>
/// Setter for zoom.
/// Also updates the values for scale and viewport accordingly.
/// View is now  dirty.
/// </summary>
/// <param name="z">The new zoom</param>
void View::zoomTo(Real z)
{
    zoom = z;
    scale = pow(2, zoom);
    isDirty(true);
    updateViewport();
}

/// <summary>
/// Incrementer for zoom.
/// Also updates the values for scale and viewport accordingly.
/// View is now  dirty.
/// </summary>
/// <param name="z">The amount to change the zoom by</param>
void View::zoomBy(Real dz)
{
    zoom += dz;
    scale = pow(2, zoom);
    isDirty(true);
    updateViewport();
}


/// <summary>
/// Begins a box zoom by registering the starting corner of the box
/// </summary>
/// <param name="x">Mouse x position when pressed</param>
/// <param name="y">Mouse y position when pressed</param>
void View::zoomBoxBegin(int x, int y)
{
    zoomBoxIsShown = true;
    zoomBoxStartCorner = Pixel(x, y);
    std::cout << "Zoom start: " << x << ", " << y << std::endl;
}

/// <summary>
/// Continues a box zoom by updating the ending corner of the box
/// </summary>
/// <param name="x">Mouse x position when moved</param>
/// <param name="y">Mouse y position when moved</param>
void View::zoomBoxContinue(int x, int y)
{
    if (zoomBoxIsShown)
    {
        // Maintain aspect ratio by scaling to the largest rectangle of the
        // correct aspect ratio that could fit the zoombox within it
        
        float w = (float)(x - zoomBoxStartCorner.x);
        float h = (float)(y - zoomBoxStartCorner.y);
        
        if (aspectRatio * abs(h) > abs(w))
            w = abs(h) * aspectRatio * (signbit(w) ? -1 : 1);
        else
            h = abs(w) / aspectRatio * (signbit(h) ? -1 : 1);

        // Set the end corner by moving (w,h) from start.
        zoomBoxEndCorner = Pixel(
            zoomBoxStartCorner.x + (int)w, 
            zoomBoxStartCorner.y + (int)h);
        
        // Set the shape to be drawn
        zoomBoxShape.setPosition(
            fmin(zoomBoxStartCorner.x, zoomBoxEndCorner.x),
            fmin(zoomBoxStartCorner.y, zoomBoxEndCorner.y));
        zoomBoxShape.setSize(sf::Vector2f(abs(w), abs(h)));
    }
        
}

/// <summary>
/// Finishes a box zoom by setting the ending corner of the box and updating
/// the viewport to be centred on the box centre and scaled to the box height.
/// </summary>
/// <param name="x">Mouse x position when released</param>
/// <param name="y">Mouse y position when released</param>
void View::zoomBoxEnd(int x, int y)
{
    // Ensure position is up to date. Not completely necessary.
    zoomBoxContinue(x, y);
    
    // Get height (2*scale) of the new view
    // Zooming to 0 scale would cause problems
    double h = abs(zoomBoxEndCorner.y - zoomBoxStartCorner.y);
    if (h > 0)
    {
        // Move to centre of zoom box and adjust scale. These methods will also
        // update the view port and set the view as dirty.
        moveTo(complexAtPixel((zoomBoxStartCorner + zoomBoxEndCorner) / 2));
        setScale(scale * h / screenSize.y);
    }
    
    // Prevent the zoom box from showing on mouse pressed on subsequent zooms.
    zoomBoxIsShown = false;
    zoomBoxShape.setSize(sf::Vector2f(0, 0));
}

/// <summary>
/// Cancels the box zoom so no changes will be made to the view.
/// </summary>
void View::zoomBoxCancel() { zoomBoxIsShown = false; }

/// <summary>
/// Getter for zoomBoxIsShown
/// </summary>
/// <returns>True if the zoombox is shown implying mouse is pressed</returns>
bool View::getZoomBoxIsShown() { return zoomBoxIsShown; }

/// <summary>
/// Getter for zoomBoxShape
/// </summary>
/// <returns>The shape of the zoom box, ready to be drawn</returns>
sf::RectangleShape View::getZoomBoxShape() { return zoomBoxShape; }


/// <summary>
/// Getter for the viewport rectangle in the complex plane.
/// </summary>
/// <returns>Viewport rectangle in the complex plane</returns>
ComplexRect View::getViewport() { return rect; }

/// <summary>
/// Getter for the position of the top left corner of the viewport in the
/// complex plane
/// </summary>
/// <returns></returns>
Complex View::getViewportPosition() { return Complex(rect.left, rect.top); }

/// <summary>
/// Getter for the diagonal of the viewport in the complex plane
/// </summary>
/// <returns></returns>
Complex View::getViewportSize() { return Complex(rect.width, rect.height); }

/// <summary>
/// Setter for the viewport rectangle.
/// Also updates the scale/zoom and centre position accordingly.
/// View is now dirty.
/// </summary>
/// <param name="r">The new value for the viewport in the complex plane</param>
void View::setViewport(ComplexRect r)
{
    rect = r;
    scale = 0.5 * r.height;
    zoom = log2(scale);
    centre.x = r.left + 0.5 * r.width;
    centre.y = r.top  + 0.5 * r.height;
    isDirty(true);
}

/// <summary>
/// Recalculates the viewport rectangle with the current values for scale and
/// centre position.
/// </summary>
void View::updateViewport()
{
    Complex scaleVector(scale * aspectRatio, scale);
    rect = ComplexRect(centre - scaleVector, scaleVector);
}


/// <summary>
/// Getter for centre position.
/// </summary>
/// <returns>The centre position in the complex plane</returns>
Complex View::getCentre() { return centre; }

/// <summary>
/// Vector incrementer for centre position.
/// Moves the centre position in the complex plane by the provided complex
/// number.
/// Also updates the viewport accordingly.
/// The view is now dirty.
/// </summary>
/// <param name="displacement">The displacement to move the view by in the
/// complex plane</param>
void View::moveBy(Complex displacement)
{
    centre += displacement * scale;
    isDirty(true);
    updateViewport();
}

/// <summary>
/// Incrementer for centre position x and y coordinates.
/// Moves the centre position in the complex plane by the real amount dx and
/// imaginary amount dy.
/// Also updates the viewport accordingly.
/// The view is now dirty.
/// </summary>
/// <param name="displacement">The displacement to move the view by in the
/// complex plane</param>
void View::moveBy(Real dx, Real dy)
{
    centre.x += dx * scale;
    centre.y += dy * scale;
    isDirty(true);
    updateViewport();
}

/// <summary>
/// Setter for centre position.
/// Moves the centre position in the complex plane to the provided complex
/// number.
/// Also updates the viewport accordingly.
/// The view is now dirty.
/// </summary>
/// <param name="position"></param>
void View::moveTo(Complex position)
{
    centre = position;
    isDirty(true);
    updateViewport();
}

/// <summary>
/// Setter for centre position x and y coordinates.
/// Moves the centre position in the complex plane to the real value x and
/// imaginary value y.
/// Also updates the viewport accordingly.
/// The view is now dirty.
/// </summary>
/// <param name="displacement">The displacement to move the view by in the
/// complex plane</param>
void View::moveTo(Real x, Real y)
{
    centre.x = x;
    centre.y = y;
    isDirty(true);
    updateViewport();
}


/// <summary>
/// Converts pixel coordinates in the screen, p(x, y), to the complex number
/// at that position in the view.
/// </summary>
/// <param name="p">Pixel integer vector</param>
/// <returns>Complex number at the specified pixel on the screen</returns>
Complex View::complexAtPixel(Pixel p)
{
    // Map x from range 0:W to range centre.x ± scale*aspectRatio
    // Map y from range 0:H to range centre.y ± scale
    return centre + scale * Complex(2 * p - screenSize) / (Real)screenSize.y;
}

/// <summary>
/// Converts pixel coordinates in the screen, (x, y), to the complex number
/// at that position in the view.
/// </summary>
/// <param name="x">Pixel coordinate x</param>
/// <param name="y">Pixel coordinate y</param>
/// <returns>Complex number at the specified pixel on the screen</returns>
Complex View::complexAtPixel(int x, int y)
{
    // Map x from range 0:W to range centre.x ± scale*aspectRatio
    // Map y from range 0:H to range centre.y ± scale
    return Complex(centre.x + (scale / screenSize.y) * (Real)(2 * x - screenSize.x),
                   centre.y + (scale / screenSize.y) * (Real)(2 * y - screenSize.y));
}


/// <summary>
/// Converts a given complex number to its pixel position relative to the view.
/// </summary>
/// <param name="z">Complex number relative to the view</param>
/// <returns>Pixel coord at the specified complex number in the view</returns>
Pixel View::pixelAtComplex(Complex z)
{
    return Pixel(0.5 * ((z - centre) * (screenSize.y / scale) + Complex(screenSize)));
    //return pixelAtComplex(z.x, z.y);
}

/// <summary>
/// Converts a given complex number in the view with real part x and imaginary
/// part y to its pixel position relative to the view.
/// </summary>
/// <param name="z">Complex number relative to the view</param>
/// <returns>Pixel coord at the specified complex number in the view</returns>
Pixel View::pixelAtComplex(Real x, Real y)
{
    // Map x from range c.x ± s(W/H) to range 0:W
    // Map y from range c.y ± s to range 0:H
    return Pixel(0.5 * ((x - centre.x) * (screenSize.y / scale) + screenSize.x),
                 0.5 * ((y - centre.y) * (screenSize.y / scale) + screenSize.y));
}


/// <summary>
/// Operator overload for <<
/// Allows the view object to be printed to standard output streams.
/// The format for this string is given by
/// ( %+e, %+e ) @ %+e -> [ %+e, %+e, %e, %e ]
/// Where the first three parameters are the centre coordinates and zoom,
/// and the last four parameters are the viewport rectangle.
/// This string will always be 120 characters in length.
/// </summary>
/// <example>
/// <code>
/// View view;
/// std::cout << view << std::endl; // Prints formatted string to console.
/// </code>
/// </example>
/// <param name="stream">Any standard output stream</param>
/// <param name="v">An instance of View</param>
/// <returns>The output stream so that further data may be inserted into the
/// output stream</returns>
std::ostream& operator<<(std::ostream &stream, const View &v)
{
    char str[120];
    sprintf_s(str, "( %+e, %+e ) @ %+e -> [ %+e, %+e, %e, %e ]",
        v.centre.x, v.centre.y, v.zoom, 
        v.rect.left, v.rect.top, v.rect.width, v.rect.height);
    return stream << str;
}
