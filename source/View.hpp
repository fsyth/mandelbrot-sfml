#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

typedef double Real;
typedef sf::Vector2<Real> Complex;
typedef sf::Rect<Real> ComplexRect;
typedef sf::Vector2<int> Pixel;
typedef sf::Rect<int> PixelRect;

class View
{
private:
    bool dirty = true;
    Real scale = 2;         // = 2^zoom
    Real zoom = 1;          // = log2(scale)
    Complex centre;
    ComplexRect rect;       // = centre ± scale
    Pixel screenSize;
    Real aspectRatio;
    bool zoomBoxIsShown = false;
    Pixel zoomBoxStartCorner;
    Pixel zoomBoxEndCorner;
    sf::RectangleShape zoomBoxShape;
    void updateViewport();
    friend std::ostream& operator<<(std::ostream &stream, const View &v);

public:
    View(Real x = 0, Real y = 0, Real zoom = 1, 
         int screenWidth = 1, int screenHeight = 1);
    ~View();
    bool isDirty();
    void isDirty(bool d);
    Real getScale();
    void setScale(Real s);
    Real getZoom();
    void zoomTo(Real z);
    void zoomBy(Real dz);
    Complex getCentre();
    void moveBy(Complex displacement);
    void moveBy(Real dx, Real dy);
    void moveTo(Complex position);
    void moveTo(Real x, Real y);
    ComplexRect getViewport();
    Complex getViewportPosition();
    Complex getViewportSize();
    void setViewport(ComplexRect v);
    Complex complexAtPixel(Pixel p);
    Complex complexAtPixel(int x, int y);
    Pixel pixelAtComplex(Complex z);
    Pixel pixelAtComplex(Real x, Real y);
    void zoomBoxBegin(int x, int y);
    void zoomBoxContinue(int x, int y);
    void zoomBoxEnd(int x, int y);
    void zoomBoxCancel();
    bool getZoomBoxIsShown();
    sf::RectangleShape getZoomBoxShape();
};
