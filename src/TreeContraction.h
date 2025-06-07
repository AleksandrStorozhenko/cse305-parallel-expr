#ifndef TREECONTRACTION_H
#define TREECONTRACTION_H

#include "Node.h"
#include "ThreadPool.h"
#include <vector>

class TreeContraction
{
public:
    static void schedule_contract(const std::vector<Node::Ptr>& nodes, std::size_t st, std::size_t en, SimplePool& pool)
    {
        for (std::size_t i = st; i < en; ++i){
            if (nodes[i] && !nodes[i]->isDone()){
                pool.push([&node = nodes[i]]() { node->contract(); });
            }
        }
    }

    static std::size_t TreeContract(std::vector<Node::Ptr> nodes,
                                    Node::Ptr root,
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
                          static_cast<std::size_t>(std::min(i + stride, n)),
                          std::ref(pool));
            pool.waitEmpty(); // stronger barrier
        }
        return 0;
    }
};

#endif

