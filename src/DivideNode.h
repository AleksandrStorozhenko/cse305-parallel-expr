#ifndef DIVIDENODE_H
#define DIVIDENODE_H

#include "Node.h"
#include <cassert>

class DivideNode : public Node {
    void on_rake_left(double x) override {
        if (lin_frac.was_set()) value = lin_frac.eval(x);
        else lin_frac = LinearFractional(0, x, 1, 0);
    }
    void on_rake_right(double x) override {
        if (lin_frac.was_set()) value = lin_frac.eval(x);
        else lin_frac = LinearFractional(1, 0, 0, x);
    }

public:
    DivideNode(const Ptr& l, const Ptr& r) : Node(l, r) {}

    double compute() override {
        double first = children()[0]->compute();
        double second = children()[1]->compute();
        assert(second != 0 && "DivideNode: division by zero");
        value = first / second;
        return *value;
    }
};

#endif

