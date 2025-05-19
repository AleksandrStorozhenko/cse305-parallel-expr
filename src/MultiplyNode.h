#ifndef MULTIPLYNODE_H
#define MULTIPLYNODE_H

#include "Node.h"

class MultiplyNode: public Node {
public:
    MultiplyNode(Node* left, Node* right) : Node(left, right) { }

    double compute(){
        double multiply = 1;
        for(auto& son: sons){
            multiply *= son->compute();
        }
        return multiply;
    }
};

#endif