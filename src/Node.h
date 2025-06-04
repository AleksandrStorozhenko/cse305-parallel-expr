#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <optional>
#include "LinearFractional.h"

class TreeContraction;

class Node
{
private:
    virtual void on_rake_left(double val) = 0;
    virtual void on_rake_right(double val) = 0;

protected:
    Node* parent{nullptr};
    std::unique_ptr<Node> left{nullptr};
    std::unique_ptr<Node> right{nullptr};

    std::atomic<int> num_children{0};
    std::mutex mutex;
    
    bool is_left{false};
    LinearFractional lin_frac;

    std::atomic<bool> done{false};
    std::atomic_flag busy = ATOMIC_FLAG_INIT;

public:
    std::optional<double> value;

    Node() = default;

    Node(Node* _left, Node* _right) : parent(nullptr)
    {
        if(_left){
            left.reset(_left);
            _left->is_left = true;
            _left->parent = this;
            num_children++;
        }
        if(_right){
            right.reset(_right);
            _right->is_left = false;
            _right->parent = this;
            num_children++;
        }
    }

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    virtual ~Node() = default;

    std::size_t degree() const { return num_children.load(); }
    bool isLeaf() const { return num_children.load() == 0; }
    bool isDone() const { return done.load(); }

    std::vector<Node*> children() {
        std::vector<Node*> c;
        if(left) c.push_back(left.get());
        if(right) c.push_back(right.get());
        return c;
    }
    
    void contract(){
        if(done.load()) return;
        if(busy.test_and_set()) return;

        std::unique_lock<std::mutex> lk_self(mutex);

        std::unique_ptr<Node> self_owner;

        if(num_children.load() == 0 && parent){
            lk_self.unlock();
            std::unique_lock<std::mutex> lk_par(parent->mutex,std::defer_lock);
            std::lock(lk_self,lk_par);

            if(is_left){
                parent->on_rake_left(*value);
                self_owner = std::move(parent->left);
            }else{
                parent->on_rake_right(*value);
                self_owner = std::move(parent->right);
            }

            parent->num_children.fetch_sub(1,std::memory_order_acq_rel);
            done.store(true,std::memory_order_release);
        }
        else if(num_children.load() == 1 && parent){
            Node* son = children()[0];

            lk_self.unlock();
            std::unique_lock<std::mutex> lk_par(parent->mutex,std::defer_lock);
            std::unique_lock<std::mutex> lk_son(son->mutex,std::defer_lock);
            std::lock(lk_self,lk_par,lk_son);

            if(parent->num_children.load() == 1){
                parent->lin_frac = parent->lin_frac.compose(lin_frac);
                
                std::unique_ptr<Node> child_owner =
                    left ? std::move(left) : std::move(right);
                if(is_left){
                    self_owner = std::move(parent->left);
                    parent->left = std::move(child_owner);
                    parent->left->is_left = true;
                }else{
                    self_owner = std::move(parent->right);
                    parent->right = std::move(child_owner);
                    parent->right->is_left = false;
                }
                parent->left ? parent->left->parent = parent : parent->right->parent = parent;
                done.store(true,std::memory_order_release);
            }
        }

        busy.clear(std::memory_order_release);
    }
    
    virtual double compute() = 0;
};

#endif
