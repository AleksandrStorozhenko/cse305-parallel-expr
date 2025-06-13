#ifndef TREECONTRACTION_H
#define TREECONTRACTION_H

#include "Node.h"
#include "ThreadPool.h"
#include <vector>
#include <latch>

class TreeContraction
{
public:
    static void schedule_contract(const std::vector<Node::Ptr>& nodes, std::size_t st, std::size_t en, std::latch& ltch)
    {
        for (std::size_t i = st; i < en; ++i){
            if (nodes[i] && !nodes[i]->isDone()){
                nodes[i]->contract();
                // std::cout<<"Node i = "<<i<<" not done"<<std::endl;
            }
            ltch.count_down();
        }
    }

    static void TreeContract(const std::vector<Node::Ptr>& nodes,
                                    const Node::Ptr& root,
                                    int num_threads,
                                    SimplePool& pool)
    {
        int n = static_cast<int>(nodes.size());
        int stride = n / num_threads + 1;
        std::cout<<"Starting tree contract with n ="<<n<<" num_t = "<<num_threads<<std::endl;
        while (root->degree() > 0) {
            std::latch ltch(n);
            std::cout<<"Root deg = "<<root->degree()<<std::endl;
            for (int i = 0; i < n; i += stride)
                pool.push(schedule_contract,
                          std::cref(nodes),
                          static_cast<std::size_t>(i),
                          static_cast<std::size_t>(std::min(i + stride, n)),
                          std::ref(ltch));
            ltch.wait();
        }
    }
};

#endif

