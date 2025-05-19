#ifndef PLUSNODE_H
#define PLUSNODE_H

#include "Node.h"

class PlusNode: public Node {
public:
    PlusNode(Node* left, Node* right) : Node(left, right) { }
    
    double compute(){
        double sum = 0;
        for(auto& son: sons){
            sum += son->compute();
        }
        return sum;
    }
};

#endif