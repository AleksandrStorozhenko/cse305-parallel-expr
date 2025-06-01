#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include "LinearFractional.h"
#include <mutex>
#include <atomic>
#include <thread>

class TreeContraction;

class Node
{

protected:
    Node* parent;
    Node* left;
    Node* right;


    int num_children;
    std::mutex mutex;
    
    bool is_left{false};
    LinearFractional lin_frac;
    double value;
public:
    
    Node() : parent(nullptr), left(nullptr), right(nullptr), num_children(0) {}
   
    Node(Node* _left, Node* _right) : parent(nullptr), left(_left), right(_right)
    {
        num_children = 0;

        if (left){ 
            left->is_left = true;
            left->parent = this;
            num_children++;
        }
        
        if (right){ 
            right->is_left = false;
            right->parent = this;
            num_children++;
        }
    }

    virtual ~Node()
    {
        for (auto* c:children()) delete c;
    }

    std::size_t degree() const { return num_children; }

    bool isLeaf() const { return num_children == 0; }

    std::vector<Node*> children() {
        std::vector<Node*> c{};
        if(left)
            c.push_back(left);
        if(right)
            c.push_back(right);
        return c;
    }
    
    // void removeChild(Node* child)
    // {
    //     children.erase(std::remove(children.begin(), children.end(), child),
    //     children.end());
    // }

    // void replaceChild(Node* oldChild, Node* newChild)
    // {
    //     for (auto& c : children)
    //     {
    //         if (c == oldChild)
    //         {
    //             c = newChild;
    //             if (newChild) newChild->parent = this;
    //             if (oldChild) oldChild->parent = nullptr;
    //             return;
    //         }
    //     }
    // }

    void contract(){
        //root can NOT be invalidated 

        std::unique_lock<std::mutex> lk_self(mutex);
        if(num_children == 0 && parent){
            //to avoid deadlock
            lk_self.unlock();
            std::unique_lock<std::mutex> lk_par(parent->mutex, std::defer_lock);
            std::lock(lk_self, lk_par);

            //rake
            if(is_left)
                parent->on_rake_left(value);
            else
                parent->on_rake_right(value);
            delete this;
        }
        else if(num_children == 1 && parent){

            std::unique_lock<std::mutex> lk_par(parent->mutex);

            if(parent->num_children == 1){
                //contract
                parent->lin_frac = parent->lin_frac.compose(lin_frac);

                if(parent->parent == nullptr){
                    //parent is root
                    //disconnect & delete
                    parent->left = parent->right = nullptr;
                    delete parent;

                    parent = nullptr;
                }
                else{
                    //parent is not root;
                    std::lock_guard<std::mutex> lk_great_par(parent->parent->mutex);
                    auto great_par = parent->parent;

                    if(parent->is_left){
                        great_par->left = this;
                    }
                    else{
                        great_par->right = this;
                    }

                    //disconnect & delete
                    parent->left = parent->right = nullptr;
                    delete parent;

                    parent = great_par;
                }
            }
        }
    }

    //for some operations (eg division) order matters
    virtual void on_rake_left(double val) = 0;
    virtual void on_rake_right(double val) = 0;

    virtual double compute() = 0;
};

#endif