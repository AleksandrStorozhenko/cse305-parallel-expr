#ifndef PLUSNODE_H
#define PLUSNODE_H

#include "Node.h"

class PlusNode: public Node {
    void on_rake(double x){
        if(lin_frac.was_set()){
            value = lin_frac.eval(x);
        }
        else{
            lin_frac = LinearFractional(1, x, 0, 1);   // (y -> x + y)
        }
    }
    
    void on_rake_left(double x) override {
        on_rake(x);
    }

    void on_rake_right(double x) override {
        on_rake(x);
    }

public:
    PlusNode(Node* left, Node* right) : Node(left, right) { }
    
    double compute() override {
        double sum = 0;
        for(auto& son: children()){
            sum += son->compute();
        }
        value = sum;
        return sum;
    }
};

#endif