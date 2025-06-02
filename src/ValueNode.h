#ifndef VALUENODE_H
#define VALUENODE_H

#include "Node.h"

class ValueNode: public Node {
private:

    void on_rake_left(double x) override {}
    void on_rake_right(double x) override {}

public:
    
    ValueNode(double _value) : Node(){
        value = _value;
    }

    ValueNode(double _value, Node* left, Node* right): Node(left, right) {
        value = _value;
    }


    double compute() override {
        return *value;
    }

};

#endif