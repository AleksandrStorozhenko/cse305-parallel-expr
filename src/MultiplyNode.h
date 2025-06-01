#ifndef MULTIPLYNODE_H
#define MULTIPLYNODE_H

#include "Node.h"

class MultiplyNode: public Node {
private: 
    void on_rake(double x){
        if(lin_frac.was_set()){
            value = lin_frac.eval(x);
        }
        else{
            lin_frac = LinearFractional(x, 0, 0, 0);
        }
    }
    void on_rake_left(double x){
        on_rake(x);
    }
    void on_rake_right(double x){
        on_rake(x);
    }

public:
    MultiplyNode(Node* l, Node* r) : Node(l, r) {}
    double compute() override
    {
        double prod = 1;
        for (auto* c : children()) prod *= c->compute();
        value = prod;
        return prod;
    }
};

#endif