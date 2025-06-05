#ifndef VALUENODE_H
#define VALUENODE_H

#include "Node.h"

class ValueNode : public Node {
    void on_rake_left(double) override {}
    void on_rake_right(double) override {}

public:
    explicit ValueNode(double v) : Node() { value = v; }
    double compute() override { return *value; }
};

#endif

