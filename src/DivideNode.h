#ifndef DIVIDENODE_H
#define DIVIDENODE_H

#include "Node.h"
#include <cassert>

class DivideNode: public Node {

void on_rake_left(double x){
    if(lin_frac.was_set()){
        value = lin_frac.eval(x);
    }
    else{
        lin_frac = LinearFractional(0, x, 1, 0);
    }
}
void on_rake_right(double x){
    if(lin_frac.was_set()){
        value = lin_frac.eval(x);
    }
    else{
        lin_frac = LinearFractional(1, 0, 0, x);
    }
}
public:
    DivideNode(Node* left, Node* right) : Node(left, right) { }

    double compute(){
        double first = left->compute();
        double second = right->compute();

        value = first/second;
        return *value;
    }
};

#endif