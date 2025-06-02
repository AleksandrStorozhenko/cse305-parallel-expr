#ifndef LINEARFRACTIONAL_H
#define LINEARFRACTIONAL_H

#include <iostream>
#include <cassert>

//Utility class for handling linear fractional operations

class LinearFractional {

private:
    double a, b, c, d;
    bool valid; // has the matrix been initialized ?

public:
    LinearFractional() : a(0), b(0), c(0), d(0), valid(false) {}

    LinearFractional(double _a, double _b, double _c, double _d)
        : a(_a), b(_b), c(_c), d(_d), valid(true) {}


    // double eval(double x) {
    double eval(double x) const {
        assert(c * x + d != 0);
        return (a * x + b) / (c * x + d);
    }

    LinearFractional compose(const LinearFractional& other) const {
        return LinearFractional(a * other.a + b * other.c,  a * other.b + b * other.d,
                                c * other.a + d * other.c,  c * other.b + d * other.d);
    } 

    bool was_set() const { return valid; }
};



#endif