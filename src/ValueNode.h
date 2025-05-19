#ifndef VALUENODE_H
#define VALUENODE_H

#include "Node.h"

class ValueNode: public Node {
    double value;
public:
    
    ValueNode(double _value) : Node(), value(_value) {}
    ValueNode(double _value, Node* left, Node* right): Node(left, right), value(_value) {}

    double compute(){
        return value;
    }
};

#endif