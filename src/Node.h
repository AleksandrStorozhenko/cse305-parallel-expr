#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <optional>
#include "LinearFractional.h"

class Node : public std::enable_shared_from_this<Node>
{
public:
    using Ptr = std::shared_ptr<Node>;

private:
    std::weak_ptr<Node> parent;
    Ptr left{nullptr};
    Ptr right{nullptr};

    std::atomic<int> num_children{0};
    std::mutex mutex;
    std::atomic<bool> done{false};

protected:
    bool is_left{false};
    LinearFractional lin_frac;

    virtual void on_rake_left(double) = 0;
    virtual void on_rake_right(double) = 0;

public:
    std::optional<double> value;

    Node() = default;

    Node(const Ptr& l, const Ptr& r)
    {
        if (l) {
            left = l;
            left->is_left = true;
            num_children++;
        }
        if (r) {
            right = r;
            right->is_left = false;
            num_children++;
        }
    }

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    virtual ~Node() = default;

    void set_parent(const Ptr& p) { parent = p; }

    int degree() const { return num_children.load(); }
    bool isLeaf() const { return num_children.load() == 0; }
    bool isDone() const { return done.load(); }

    std::vector<Ptr> children() const
    {
        std::vector<Ptr> v;
        v.reserve(2);
        if (left) v.push_back(left);
        if (right) v.push_back(right);
        return v;
    }

    void contract()
    {
        if (done.load()) return;
        // std::unique_lock lk_self(mutex, std::try_to_lock);
        // if (!lk_self.owns_lock()) return;
        // std::cout<<"Own lock for node = "<<this<<std::endl;
        //std::cout<<"Inside not done contract for node = "<<this<<std::endl;

        if (num_children.load() == 0 && !parent.expired()) {
            auto p = parent.lock();
            // lk_self.release();
            //std::cout<<"In rake; acquiring this & parent lock; this = "<<this<<"parent = "<<p<<std::endl;

            std::scoped_lock lk_par(mutex, p->mutex);

            if(!(isParent(p) && num_children.load() == 0 && p->degree() >= 1 && !isDone())){
                return;
            }
            //std::cout<<"In rake; acquired; this = "<<this<<std::endl;

            if (is_left) {
                p->on_rake_left(*value);
                p->left.reset();
            } else {
                p->on_rake_right(*value);
                p->right.reset();
            }
            p->num_children.fetch_sub(1);
            //std::cout<<"P now has "<<p->num_children.load()<<std::endl;
            done.store(true);
        }
        else if (num_children.load() == 1 && !parent.expired() && parent.lock()->num_children.load() == 1) {
            auto p = parent.lock();
            Ptr son = left ? left : right;

            //std::cout<<"In compress; acquiring this & parent & son lock; this = "<<this<<std::endl;

            std::scoped_lock lk_other(mutex, p->mutex, son->mutex);

            if(!(isParent(p) && isSon(son) && degree() == 1 && p->degree() == 1 && !isDone())){
                return;
            }
            //std::cout<<"In conttract; acquired; this = "<<this<<std::endl;

            p->lin_frac = p->lin_frac.compose(lin_frac);

            if (is_left) {
                p->left = son;
                son->is_left = true;
            } else {
                p->right = son;
                son->is_left = false;
            }
            son->parent = p;

            num_children.fetch_sub(1);
            done.store(true);
        
        }
        //std::cout<<"Node has deg = "<<num_children.load()<<std::endl;
    }

    virtual double compute() = 0;

    bool isParent(const Ptr& potential_parent) const {
        return !parent.expired() && parent.lock() == potential_parent;
    }

    bool isSon(const Ptr& potential_son) const {
        return potential_son && (left == potential_son || right == potential_son);
    }
};

#endif

