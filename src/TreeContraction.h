#ifndef TREECONTRACTION_H
#define TREECONTRACTION_H

#include "Node.h"
#include "ThreadPool.h"
#include <vector>

class TreeContraction
{
public:
    static void schedule_contract(const std::vector<Node::Ptr>& nodes, std::size_t st, std::size_t en)
    {
        for (std::size_t i = st; i < en; ++i)
            if (nodes[i] && !nodes[i]->isDone())
                nodes[i]->contract(); // executed by worker
    }

    static std::size_t TreeContract(const std::vector<Node::Ptr>& nodes,
                                    const Node::Ptr& root,
                                    int num_threads,
                                    SimplePool& pool)
    {
        int n = static_cast<int>(nodes.size());
        int stride = n / num_threads + 1;

        while (root->degree() > 0) {
            for (int i = 0; i < n; i += stride)
                pool.push(schedule_contract,
                          std::cref(nodes),
                          static_cast<std::size_t>(i),
                          static_cast<std::size_t>(std::min(i + stride, n)));
                          
            pool.waitEmpty(); 

        }
        pool.waitIdle();
        return 0;
    }
};

#endif

