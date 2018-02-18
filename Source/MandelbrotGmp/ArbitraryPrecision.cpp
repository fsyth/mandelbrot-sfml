#include "ArbitraryPrecision.hpp"

/// <summary>
/// Class wrapper around GNU Multi Precision (GMP)
/// featuring operator overloads so that doubles and arbitrary precision
/// numbers can be used interchangeably outside this class.
/// </summary>
/// <param name="initialValue"></param>
ArbitraryPrecision::ArbitraryPrecision(double initialValue)
{
    mpf_init(m_value);
    mpf_set_d(m_value, initialValue);
}

ArbitraryPrecision::ArbitraryPrecision(double initialValue, unsigned long long bits)
{
    mpf_init2(m_value, bits);
    mpf_set_d(m_value, initialValue);
}

ArbitraryPrecision::ArbitraryPrecision(const ArbitraryPrecision& copy)
{
    mpf_init(m_value);
    mpf_set(m_value, copy.m_value);
}

ArbitraryPrecision::~ArbitraryPrecision()
{
    mpf_clear(m_value);
}

unsigned long long ArbitraryPrecision::getPrecision()
{
    return mpf_get_prec(m_value);
}

void ArbitraryPrecision::setPrecision(unsigned long long bits)
{
    mpf_set_prec(m_value, bits);
}

ArbitraryPrecision& ArbitraryPrecision::operator=(const ArbitraryPrecision& rhs)
{
    if (this == &rhs)
        return *this;

    mpf_set(m_value, rhs.m_value);

    return *this;
}

ArbitraryPrecision ArbitraryPrecision::operator-() const
{
    ArbitraryPrecision result;
    mpf_neg(result.m_value, this->m_value);
    return result;
}

ArbitraryPrecision operator+(const ArbitraryPrecision& a, const ArbitraryPrecision& b)
{
    ArbitraryPrecision result;
    mpf_add(result.m_value, a.m_value, b.m_value);
    return result;
}

ArbitraryPrecision operator-(const ArbitraryPrecision& a, const ArbitraryPrecision & b)
{
    ArbitraryPrecision result;
    mpf_sub(result.m_value, a.m_value, b.m_value);
    return result;
}

ArbitraryPrecision operator*(const ArbitraryPrecision& a, const ArbitraryPrecision& b)
{
    ArbitraryPrecision result;
    mpf_mul(result.m_value, a.m_value, b.m_value);
    return result;
}

ArbitraryPrecision operator/(const ArbitraryPrecision& a, const ArbitraryPrecision& b)
{
    ArbitraryPrecision result;
    mpf_div(result.m_value, a.m_value, b.m_value);
    return result;
}

ArbitraryPrecision& ArbitraryPrecision::operator+=(const ArbitraryPrecision& b)
{
    mpf_add(this->m_value, this->m_value, b.m_value);
    return *this;
}

ArbitraryPrecision& ArbitraryPrecision::operator-=(const ArbitraryPrecision & b)
{
    mpf_sub(this->m_value, this->m_value, b.m_value);
    return *this;
}

ArbitraryPrecision& ArbitraryPrecision::operator*=(const ArbitraryPrecision& b)
{
    mpf_mul(this->m_value, this->m_value, b.m_value);
    return *this;
}

ArbitraryPrecision& ArbitraryPrecision::operator/=(const ArbitraryPrecision& b)
{
    mpf_div(this->m_value, this->m_value, b.m_value);
    return *this;
}

bool operator>(const ArbitraryPrecision& a, const ArbitraryPrecision& b)
{
    return mpf_cmp(a.m_value, b.m_value) > 0;
}

bool operator<(const ArbitraryPrecision& a, const ArbitraryPrecision& b)
{
    return mpf_cmp(a.m_value, b.m_value) < 0;
}

bool operator>=(const ArbitraryPrecision& a, const ArbitraryPrecision& b)
{
    return mpf_cmp(a.m_value, b.m_value) >= 0;
}

bool operator<=(const ArbitraryPrecision& a, const ArbitraryPrecision& b)
{
    return mpf_cmp(a.m_value, b.m_value) <= 0;
}

bool operator==(const ArbitraryPrecision& a, const ArbitraryPrecision& b)
{
    return mpf_cmp(a.m_value, b.m_value) == 0;
}

bool operator!=(const ArbitraryPrecision& a, const ArbitraryPrecision& b)
{
    return mpf_cmp(a.m_value, b.m_value) != 0;
}

bool operator>(const ArbitraryPrecision & a, const double b)
{
    return mpf_cmp_d(a.m_value, b) > 0;
}

bool operator<(const ArbitraryPrecision & a, const double b)
{
    return mpf_cmp_d(a.m_value, b) < 0;
}

bool operator>=(const ArbitraryPrecision & a, const double b)
{
    return mpf_cmp_d(a.m_value, b) >= 0;
}

bool operator<=(const ArbitraryPrecision & a, const double b)
{
    return mpf_cmp_d(a.m_value, b) <= 0;
}

bool operator==(const ArbitraryPrecision & a, const double b)
{
    return mpf_cmp_d(a.m_value, b) == 0;
}

bool operator!=(const ArbitraryPrecision & a, const double b)
{
    return mpf_cmp_d(a.m_value, b) != 0;
}

ArbitraryPrecision::operator double() const
{
    return mpf_get_d(this->m_value);
}

ArbitraryPrecision::operator float() const
{
    return static_cast<float>(mpf_get_d(this->m_value));
}

ArbitraryPrecision::operator long() const
{
    return static_cast<long>(mpf_get_si(this->m_value));
}

ArbitraryPrecision::operator int() const
{
    return static_cast<int>(mpf_get_si(this->m_value));
}

ArbitraryPrecision abs(const ArbitraryPrecision& a)
{
    ArbitraryPrecision result;
    mpf_abs(result.m_value, a.m_value);
    return result;
}

ArbitraryPrecision pow(const ArbitraryPrecision& base, unsigned long power)
{
    ArbitraryPrecision result;
    mpf_pow_ui(result.m_value, base.m_value, power);
    return result;
}
