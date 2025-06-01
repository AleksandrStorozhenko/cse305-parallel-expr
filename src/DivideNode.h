#ifndef DIVIDENODE_H
#define DIVIDENODE_H

#include "Node.h"
#include <cassert>

class DivideNode: public Node {
public:
    DivideNode(Node* left, Node* right) : Node(left, right) { }

    double compute(){
        return 0;
        // double first = sons[0]->compute();
        // double second = sons[1]->compute();
     
        // return first/second;
    }
};

#endif