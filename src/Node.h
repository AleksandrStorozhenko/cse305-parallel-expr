#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>

class Node {
protected:
    Node* parent; //NULL marks root
    std::vector<Node*> sons; // order matters

public:
    virtual double compute() = 0;
    Node() : parent(NULL), sons({}) {}
    Node(Node* left, Node* right): parent(NULL), sons({left, right}) {
        left->parent = this;
        right->parent = this;
    }

    virtual ~Node() {
        for(auto& son: sons){
            delete son;
        }
    }
};

#endif