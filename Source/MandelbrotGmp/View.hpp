#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

// From project configuration preprocessor
#ifdef UseArbitraryPrecision
#include "ArbitraryPrecision.hpp"
typedef ArbitraryPrecision Real;
#else
typedef double Real;
#endif

typedef sf::Vector2<Real> Complex;
typedef sf::Rect<Real> ComplexRect;
typedef sf::Vector2<int> Pixel;
typedef sf::Rect<int> PixelRect;

class View
{
private:
    bool m_dirty = true;
    Real m_scale = 2.0;       // = 2^zoom
    Real m_zoom = 1.0;        // = log2(scale)
    Complex m_centre;
    ComplexRect m_rect;       // = centre ± scale
    Pixel m_screenSize;
    Real m_aspectRatio;
    bool m_zoomBoxIsShown = false;
    Pixel m_zoomBoxStartCorner;
    Pixel m_zoomBoxEndCorner;
    sf::RectangleShape m_zoomBoxShape;

    void updateViewport();
    friend std::ostream& operator<<(std::ostream& out, const View& v);

public:
    View(Real x = 0, Real y = 0, Real zoom = 1,
         int screenWidth = 1, int screenHeight = 1);
    ~View();
    void resizeScreen(int screenWidth, int screenHeight);
    bool isDirty() const;
    void isDirty(bool d);
    Real getScale() const;
    void setScale(Real s);
    Real getZoom() const;
    void zoomTo(Real z);
    void zoomBy(Real dz);
    Complex getCentre() const;
    void moveBy(Complex displacement);
    void moveBy(Real dx, Real dy);
    void moveTo(Complex position);
    void moveTo(Real x, Real y);
    ComplexRect getViewport() const;
    Complex getViewportPosition() const;
    Complex getViewportSize() const;
    void setViewport(ComplexRect v);
    Complex complexAtPixel(Pixel p) const;
    Complex complexAtPixel(int x, int y) const;
    Pixel pixelAtComplex(Complex z) const;
    Pixel pixelAtComplex(Real x, Real y) const;
    void zoomBoxBegin(int x, int y);
    void zoomBoxContinue(int x, int y);
    void zoomBoxEnd(int x, int y);
    void zoomBoxCancel();
    bool getZoomBoxIsShown() const;
    sf::RectangleShape getZoomBoxShape() const;
};
