#ifndef LINEARFRACTIONAL_H
#define LINEARFRACTIONAL_H

#include <iostream>
#include <cassert>

//Utility class for handling linear fractional operations

class LinearFractional {

private:
    double a, b, c, d;
    // x -> a * x + b / (c * x + d)

public:
    LinearFractional() : a(0), b(0), c(0), d(0) {} // this implies it is not set yet
    LinearFractional(double _a, double _b, double _c, double _d) : a(_a), b(_b), c(_c), d(_d) {}

    double eval(double x) {
        assert(c*x + d != 0);

        return a * x + b / (c * x + d);
    }

    LinearFractional compose(const LinearFractional& other){
        return LinearFractional(a * other.a + b * other.c,  a * other.b + b * other.d,
                                c * other.a + d * other.c,  c * other.b + d * other.d);
    } 
};



#endif