#ifndef TREECONTRACTION_H
#define TREECONTRACTION_H

#include "Node.h"
#include "ThreadPool.h"
#include <vector>
#include <condition_variable>
#include <chrono>

class TreeContraction
{
public:
    static void schedule_contract(const std::vector<Node::Ptr>& nodes,
                                  std::size_t st, std::size_t en,
                                  SimplePool& pool,
                                  const Node::Ptr& root)
    {
        for (std::size_t i = st; i < en; ++i) {
            if (!nodes[i]->isDone()) {
                // std::cout<<"Pushing contract for node "<<i<<std::endl;

                pool.push([node = nodes[i]]() { node->contract(); });
            }
        }
    }

    static std::size_t TreeContract(const std::vector<Node::Ptr>& nodes,
                                    const Node::Ptr& root,
                                    int num_threads)
    {

        int n = static_cast<int>(nodes.size());
        int stride = n / num_threads + 1;

        SimplePool pool(num_threads);

        while (!root->value.has_value()) {
            std::cout<<"Root is = "<<root<<" and doesnt have value yet!"<<std::endl;

            for (int i = 0; i < n; i += stride) {
                pool.push(schedule_contract,
                          std::cref(nodes),
                          static_cast<std::size_t>(i),
                          static_cast<std::size_t>(std::min(i + stride, n)),
                          std::ref(pool),
                          std::cref(root));
            }
            std::cout<<"Waiting to be empty"<<std::endl;
            pool.waitEmpty();
        }

        pool.stop();
        return 0;
    }
};

#endif

