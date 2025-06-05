#ifndef MINUSNODE_H
#define MINUSNODE_H

#include "Node.h"

class MinusNode : public Node {
    void on_rake_left(double x) override {
        if (lin_frac.was_set()) value = lin_frac.eval(x);
        else lin_frac = LinearFractional(-1, x, 0, 1);
    }
    void on_rake_right(double x) override {
        if (lin_frac.was_set()) value = lin_frac.eval(x);
        else lin_frac = LinearFractional(1, -x, 0, 1);
    }

public:
    MinusNode(const Ptr& l, const Ptr& r) : Node(l, r) {}

    double compute() override {
        double first = children()[0]->compute();
        double second = children()[1]->compute();
        value = first - second;
        return *value;
    }
};

#endif

