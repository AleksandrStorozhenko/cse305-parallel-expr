#ifndef MINUSNODE_H
#define MINUSNODE_H

#include "Node.h"

class MinusNode: public Node {
private:
    void on_rake_left(double x){
        if(lin_frac.was_set()){
            value = lin_frac.eval(x);
        }
        else{
            // (1 * x + 0)/(0 * x + 0)
            lin_frac = LinearFractional(1, x, 0, 0);
        }
    }

    void on_rake_right(double x){
        if(lin_frac.was_set()){
            value = lin_frac.eval(x);
        }
        else{
            // (1 * -x + 0)/(0 * x + 0)
            lin_frac = LinearFractional(1, -x, 0, 0);
        }
    }

public:
    MinusNode(Node* left, Node* right) : Node(left, right) { }

    double compute() override {
        double first  = left->compute();
        double second = right->compute();
        value = first - second;
        return *value;
    }
};

#endif
