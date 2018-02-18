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
    m_screenSize = Pixel(screenWidth, screenHeight);
    m_aspectRatio = static_cast<Real>(screenWidth) / static_cast<Real>(screenHeight);
    moveTo(x, y);
    zoomTo(zoom);

    m_zoomBoxShape.setFillColor(sf::Color::Transparent);
    m_zoomBoxShape.setOutlineThickness(1);
    m_zoomBoxShape.setOutlineColor(sf::Color(0x808080));
}

/// <summary>
/// Destructor
/// </summary>
View::~View() {}


void View::resizeScreen(int screenWidth, int screenHeight)
{
    m_scale *= static_cast<double>(screenHeight) / m_screenSize.y;
    m_screenSize.x = screenWidth;
    m_screenSize.y = screenHeight;
    m_aspectRatio = static_cast<Real>(screenWidth) / static_cast<Real>(screenHeight);
    isDirty(true);
    updateViewport();
}


/// <summary>
/// Getter for dirty.
/// If the view is dirty, changes have occured to the view recently indicating
/// that a re-render is required.
/// </summary>
/// <returns>dirty</returns>
bool View::isDirty() const { return m_dirty; }

/// <summary>
/// Setter for dirty.
/// The view should be set to dirty whenever changes are made to it.
/// The view should be set to not dirty when the view is rendered.
/// </summary>
/// <param name="d">The new value for dirty</param>
void View::isDirty(bool d) { m_dirty = d; }


/// <summary>
/// Getter for m_scale.
/// </summary>
/// <returns>scale</returns>
Real View::getScale() const { return m_scale; }

/// <summary>
/// Setter for scale.
/// Also updates the values for zoom and viewport accordingly.
/// View is now dirty.
/// </summary>
/// <param name="s">The new value for scale</param>
void View::setScale(Real s)
{
    m_scale = s;
    m_zoom = log2(static_cast<double>(s));
    isDirty(true);
    updateViewport();
}


/// <summary>
/// Getter for zoom.
/// </summary>
/// <returns>zoom</returns>
Real View::getZoom() const { return m_zoom; }

/// <summary>
/// Setter for zoom.
/// Also updates the values for scale and viewport accordingly.
/// View is now  dirty.
/// </summary>
/// <param name="z">The new zoom</param>
void View::zoomTo(Real z)
{
    m_zoom = z;
    m_scale = pow(2, static_cast<double>(m_zoom));
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
    m_zoom += dz;
    //m_scale = pow(2, static_cast<double>(m_zoom));
    const Real TWO = 2.0;
    m_scale = 1 / pow(TWO, static_cast<long>(-m_zoom));
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
    m_zoomBoxIsShown = true;
    m_zoomBoxStartCorner = Pixel(x, y);
}

/// <summary>
/// Continues a box zoom by updating the ending corner of the box
/// </summary>
/// <param name="x">Mouse x position when moved</param>
/// <param name="y">Mouse y position when moved</param>
void View::zoomBoxContinue(int x, int y)
{
    if (m_zoomBoxIsShown)
    {
        // Maintain aspect ratio by scaling to the largest rectangle of the
        // correct aspect ratio that could fit the zoombox within it

        float w = (float)(x - m_zoomBoxStartCorner.x);
        float h = (float)(y - m_zoomBoxStartCorner.y);

        if (static_cast<float>(m_aspectRatio) * abs(h) > abs(w))
            w = abs(h) * static_cast<float>(m_aspectRatio) * (signbit(w) ? -1 : 1);
        else
            h = abs(w) / static_cast<float>(m_aspectRatio) * (signbit(h) ? -1 : 1);

        // Set the end corner by moving (w,h) from start.
        m_zoomBoxEndCorner = Pixel(
            m_zoomBoxStartCorner.x + static_cast<int>(w),
            m_zoomBoxStartCorner.y + static_cast<int>(h));

        // Set the shape to be drawn
        m_zoomBoxShape.setPosition(
            static_cast<float>(fmin(m_zoomBoxStartCorner.x, m_zoomBoxEndCorner.x)),
            static_cast<float>(fmin(m_zoomBoxStartCorner.y, m_zoomBoxEndCorner.y)));
        m_zoomBoxShape.setSize(sf::Vector2f(abs(w), abs(h)));
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
    Real h = abs(m_zoomBoxEndCorner.y - m_zoomBoxStartCorner.y);
    if (h > 0)
    {
        // Move to centre of zoom box and adjust scale. These methods will also
        // update the view port and set the view as dirty.
        moveTo(complexAtPixel((m_zoomBoxStartCorner + m_zoomBoxEndCorner) / 2));
        setScale(m_scale * h / m_screenSize.y);
    }

    // Prevent the zoom box from showing on mouse pressed on subsequent zooms.
    m_zoomBoxIsShown = false;
    m_zoomBoxShape.setSize(sf::Vector2f(0, 0));
}

/// <summary>
/// Cancels the box zoom so no changes will be made to the view.
/// </summary>
void View::zoomBoxCancel() { m_zoomBoxIsShown = false; }

/// <summary>
/// Getter for zoomBoxIsShown
/// </summary>
/// <returns>True if the zoombox is shown implying mouse is pressed</returns>
bool View::getZoomBoxIsShown() const { return m_zoomBoxIsShown; }

/// <summary>
/// Getter for zoomBoxShape
/// </summary>
/// <returns>The shape of the zoom box, ready to be drawn</returns>
sf::RectangleShape View::getZoomBoxShape() const { return m_zoomBoxShape; }

/// <summary>
/// Getter for the viewport rectangle in the complex plane.
/// </summary>
/// <returns>Viewport rectangle in the complex plane</returns>
ComplexRect View::getViewport() const { return m_rect; }

/// <summary>
/// Getter for the position of the top left corner of the viewport in the
/// complex plane
/// </summary>
/// <returns></returns>
Complex View::getViewportPosition() const { return Complex(m_rect.left, m_rect.top); }

/// <summary>
/// Getter for the diagonal of the viewport in the complex plane
/// </summary>
/// <returns></returns>
Complex View::getViewportSize() const { return Complex(m_rect.width, m_rect.height); }

/// <summary>
/// Setter for the viewport rectangle.
/// Also updates the scale/zoom and centre position accordingly.
/// View is now dirty.
/// </summary>
/// <param name="r">The new value for the viewport in the complex plane</param>
void View::setViewport(ComplexRect r)
{
    static const Real HALF = 0.5;
    m_rect = r;
    m_scale = HALF * r.height;
    m_zoom = log2(static_cast<double>(m_scale));
    m_centre.x = r.left + HALF * r.width;
    m_centre.y = r.top  + HALF * r.height;
    isDirty(true);
}

/// <summary>
/// Recalculates the viewport rectangle with the current values for scale and
/// centre position.
/// </summary>
void View::updateViewport()
{
    Complex scaleVector(m_scale * m_aspectRatio, m_scale);
    m_rect = ComplexRect(m_centre - scaleVector, scaleVector);
}


/// <summary>
/// Getter for centre position.
/// </summary>
/// <returns>The centre position in the complex plane</returns>
Complex View::getCentre() const { return m_centre; }

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
    m_centre += displacement * m_scale;
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
    m_centre.x += dx * m_scale;
    m_centre.y += dy * m_scale;
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
    m_centre = position;
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
    m_centre.x = x;
    m_centre.y = y;
    isDirty(true);
    updateViewport();
}


/// <summary>
/// Converts pixel coordinates in the screen, p(x, y), to the complex number
/// at that position in the view.
/// </summary>
/// <param name="p">Pixel integer vector</param>
/// <returns>Complex number at the specified pixel on the screen</returns>
Complex View::complexAtPixel(Pixel p) const
{
    // Map x from range 0:W to range centre.x ± scale*aspectRatio
    // Map y from range 0:H to range centre.y ± scale
    return m_centre + m_scale * Complex(2 * p - m_screenSize) / static_cast<Real>(m_screenSize.y);
}

/// <summary>
/// Converts pixel coordinates in the screen, (x, y), to the complex number
/// at that position in the view.
/// </summary>
/// <param name="x">Pixel coordinate x</param>
/// <param name="y">Pixel coordinate y</param>
/// <returns>Complex number at the specified pixel on the screen</returns>
Complex View::complexAtPixel(int x, int y) const
{
    // Map x from range 0:W to range centre.x ± scale*aspectRatio
    // Map y from range 0:H to range centre.y ± scale
    return Complex(m_centre.x + (m_scale / m_screenSize.y) * (2 * x - m_screenSize.x),
                   m_centre.y + (m_scale / m_screenSize.y) * (2 * y - m_screenSize.y));
}


/// <summary>
/// Converts a given complex number to its pixel position relative to the view.
/// </summary>
/// <param name="z">Complex number relative to the view</param>
/// <returns>Pixel coord at the specified complex number in the view</returns>
Pixel View::pixelAtComplex(Complex z) const
{
    static const Real HALF = 0.5;
    return Pixel(HALF * ((z - m_centre) * (m_screenSize.y / m_scale) + Complex(m_screenSize)));
}

/// <summary>
/// Converts a given complex number in the view with real part x and imaginary
/// part y to its pixel position relative to the view.
/// </summary>
/// <param name="z">Complex number relative to the view</param>
/// <returns>Pixel coord at the specified complex number in the view</returns>
Pixel View::pixelAtComplex(Real x, Real y) const
{
    // Map x from range c.x ± s(W/H) to range 0:W
    // Map y from range c.y ± s to range 0:H
    static const Real HALF = 0.5;
    return Pixel(static_cast<int>(HALF * ((x - m_centre.x) * (m_screenSize.y / m_scale) + static_cast<Real>(m_screenSize.x))),
                 static_cast<int>(HALF * ((y - m_centre.y) * (m_screenSize.y / m_scale) + static_cast<Real>(m_screenSize.y))));
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
std::ostream& operator<<(std::ostream& out, const View& v)
{
    char str[120];
    sprintf_s(str, "( %+e, %+e ) @ %+e -> [ %+e, %+e, %e, %e ]",
        static_cast<double>(v.m_centre.x),  static_cast<double>(v.m_centre.y), static_cast<double>(v.m_zoom),
        static_cast<double>(v.m_rect.left), static_cast<double>(v.m_rect.top), static_cast<double>(v.m_rect.width), static_cast<double>(v.m_rect.height));
    return out << str;
}
