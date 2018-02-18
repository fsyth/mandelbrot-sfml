#pragma once

#include <gmp.h>

class ArbitraryPrecision
{
public:
    ArbitraryPrecision(double initialValue = 0);
    ArbitraryPrecision(double initialValue, unsigned long long bits);
    ArbitraryPrecision(const ArbitraryPrecision& copy);
    ~ArbitraryPrecision();

    unsigned long long getPrecision();
    void setPrecision(unsigned long long bits);
    ArbitraryPrecision& operator=(const ArbitraryPrecision& rhs);
    ArbitraryPrecision operator-() const;
    friend ArbitraryPrecision operator+(const ArbitraryPrecision& a, const ArbitraryPrecision& b);
    friend ArbitraryPrecision operator-(const ArbitraryPrecision& a, const ArbitraryPrecision& b);
    friend ArbitraryPrecision operator*(const ArbitraryPrecision& a, const ArbitraryPrecision& b);
    friend ArbitraryPrecision operator/(const ArbitraryPrecision& a, const ArbitraryPrecision& b);
    ArbitraryPrecision& operator+=(const ArbitraryPrecision& b);
    ArbitraryPrecision& operator-=(const ArbitraryPrecision& b);
    ArbitraryPrecision& operator*=(const ArbitraryPrecision& b);
    ArbitraryPrecision& operator/=(const ArbitraryPrecision& b);
    friend bool operator>(const ArbitraryPrecision& a, const ArbitraryPrecision& b);
    friend bool operator<(const ArbitraryPrecision& a, const ArbitraryPrecision& b);
    friend bool operator>=(const ArbitraryPrecision& a, const ArbitraryPrecision& b);
    friend bool operator<=(const ArbitraryPrecision& a, const ArbitraryPrecision& b);
    friend bool operator==(const ArbitraryPrecision& a, const ArbitraryPrecision& b);
    friend bool operator!=(const ArbitraryPrecision& a, const ArbitraryPrecision& b);
    friend bool operator>(const ArbitraryPrecision& a, const double b);
    friend bool operator<(const ArbitraryPrecision& a, const double b);
    friend bool operator>=(const ArbitraryPrecision& a, const double b);
    friend bool operator<=(const ArbitraryPrecision& a, const double b);
    friend bool operator==(const ArbitraryPrecision& a, const double b);
    friend bool operator!=(const ArbitraryPrecision& a, const double b);
    explicit operator double() const;
    explicit operator float() const;
    explicit operator long() const;
    explicit operator int() const;
    friend ArbitraryPrecision abs(const ArbitraryPrecision& a);
    friend ArbitraryPrecision pow(const ArbitraryPrecision& base, unsigned long power);

private:
    mpf_t m_value;
};
