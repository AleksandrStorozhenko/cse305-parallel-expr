#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include "LinearFractional.h"
#include <mutex>
#include <atomic>
#include <thread>
#include <optional>

class TreeContraction;

class Node
{
private:
//for some operations (eg division) order matters
//moreover, invariant is: lock is held when called
    virtual void on_rake_left(double val) = 0;
    virtual void on_rake_right(double val) = 0;

protected:
    Node* parent;
    Node* left;
    Node* right;

    //int num_children; - 2 different threads may rake children at the same time
    std::atomic<int> num_children;

    std::mutex mutex;
    
    bool is_left{false};
    LinearFractional lin_frac;

    /* important fixes */
    std::atomic<bool> done{false}; // was plain bool
    std::atomic_flag busy = ATOMIC_FLAG_INIT; // guard against duplicate contract()
    /* -------------- */

public:
    std::optional<double> value;
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

    std::size_t degree() const { return num_children.load(); }

    bool isLeaf() const { return num_children.load() == 0; }

    bool isDone() const { return done.load(); }

    std::vector<Node*> children() {
        std::vector<Node*> c{};
        if(left) c.push_back(left);
        if(right) c.push_back(right);
        return c;
    }
    
    void contract(){
        //root can NOT be invalidated
        if(done.load()) return; // already finished
        if(busy.test_and_set()) return; // another thread is here

        std::unique_lock<std::mutex> lk_self(mutex);

        if(num_children.load() == 0 && parent){
            //to avoid deadlock

            lk_self.unlock();
            std::unique_lock<std::mutex> lk_par(parent->mutex, std::defer_lock);
            std::lock(lk_self, lk_par);

            //rake
            if(is_left){
                parent->on_rake_left(*value);
                parent->left = nullptr;
            }
            else{
                parent->on_rake_right(*value);
                parent->right = nullptr;
            }

            parent->num_children.fetch_sub(1, std::memory_order_acq_rel); // atomic dec
            done.store(true, std::memory_order_release);
        }

        // each thread should see the most recent count
        else if(num_children.load() == 1 && parent){
            auto son = children()[0];
            //to avoid deadlock
            lk_self.unlock();
            std::unique_lock<std::mutex> lk_par(parent->mutex, std::defer_lock);
            std::unique_lock<std::mutex> lk_son(son->mutex, std::defer_lock);

            std::lock(lk_self, lk_par, lk_son);

            if(parent->num_children.load() == 1){
                //contract

                parent->lin_frac = parent->lin_frac.compose(lin_frac);
                
                if(is_left){
                    parent->left = son;
                    son->is_left = true;
                }
                else{
                    parent->right = son;
                    son->is_left = false;
                }
                son->parent = parent;

                //disconnect
                left = right = nullptr;
                done.store(true, std::memory_order_release);
            }
        }

        busy.clear(std::memory_order_release); // allow reschedule if not done
    }
    
    virtual double compute() = 0;
};

#endif
