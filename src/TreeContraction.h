#ifndef TREECONTRACTION_H
#define TREECONTRACTION_H

#include "Node.h"
#include "ThreadPool.h"
#include <vector>
#include <condition_variable>
#include <chrono>

class TreeContraction
{
    static inline std::mutex root_mx;
    static inline std::condition_variable root_cv;
    static inline std::atomic<bool> root_ready{false};

public:
    static void schedule_contract(const std::vector<Node::Ptr>& nodes,
                                  std::size_t st, std::size_t en,
                                  SimplePool&,
                                  const Node::Ptr& root)
    {
        for (std::size_t i = st; i < en; ++i) {
            if (!nodes[i]->isDone()) {
                nodes[i]->contract();
                if (!root_ready.load(std::memory_order_acquire) &&
                    root->degree() == 0 && root->value.has_value()) {
                    root_ready.store(true, std::memory_order_release);
                    root_cv.notify_one();
                }
            }
        }
    }

    static std::size_t TreeContract(const std::vector<Node::Ptr>& nodes,
                                    const Node::Ptr& root,
                                    int num_threads)
    {
        root_ready.store(false, std::memory_order_relaxed);

        int n = static_cast<int>(nodes.size());
        int stride = n / num_threads + 1;

        SimplePool pool(num_threads);

        while (!root_ready.load(std::memory_order_acquire)) {
            for (int i = 0; i < n; i += stride)
                pool.push(schedule_contract,
                          std::cref(nodes),
                          static_cast<std::size_t>(i),
                          static_cast<std::size_t>(std::min(i + stride, n)),
                          std::ref(pool),
                          root);

            std::unique_lock lk(root_mx);
            root_cv.wait_for(lk, std::chrono::milliseconds(1),
                             [] { return root_ready.load(std::memory_order_acquire); });
        }

        pool.stop();
        return 0;
    }
};

#endif

