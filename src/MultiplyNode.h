#ifndef MULTIPLYNODE_H
#define MULTIPLYNODE_H

#include "Node.h"

class MultiplyNode: public Node {
public:
    MultiplyNode(Node* l, Node* r) : Node(l, r) {}
    double compute() override
    {
        double prod = 1;
        for (auto* c : children()) prod *= c->compute();
        return prod;
    }
};

#endif