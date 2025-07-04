#ifndef MULTIPLYNODE_H
#define MULTIPLYNODE_H

#include "Node.h"

class MultiplyNode : public Node {
    void on_rake(double x){
        if (lin_frac.was_set()) value = lin_frac.eval(x);
        else lin_frac = LinearFractional(x, 0, 0, 1);
    }
    void on_rake_left(double x) override { on_rake(x); }
    void on_rake_right(double x) override { on_rake(x); }

public:
    MultiplyNode(const Ptr& l, const Ptr& r) : Node(l, r) {}

    double compute() override {
        double prod = 1;
        for (auto& c : children()) prod *= c->compute();
        value = prod;
        return prod;
    }
};

#endif

