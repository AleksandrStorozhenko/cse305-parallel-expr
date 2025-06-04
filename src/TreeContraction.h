#ifndef TREECONTRACTION_H
#define TREECONTRACTION_H

#include "Node.h"
#include "ThreadPool.h"
#include <vector>
#include <functional>
#include <condition_variable>
#include <chrono>

class TreeContraction
{
	// helpers to detect root completion
	static inline std::mutex root_mx;
	static inline std::condition_variable root_cv;
	static inline std::atomic<bool> root_ready{false};

public:
	// contract nodes[start,end)
	static void schedule_contract(const std::vector<Node*>& nodes,
								  size_t start, size_t end,
								  SimplePool& /*unused*/,
								  Node* root)
	{
		for (size_t i = start; i < end; ++i){
			if (!nodes[i]->isDone()){
				nodes[i]->contract(); // contract node

				if (!root_ready.load(std::memory_order_acquire) &&
				    root->degree() == 0 && root->value.has_value()){
					root_ready.store(true, std::memory_order_release);
					root_cv.notify_one();
				}
			}
		}
	}

	// main entry
	static std::size_t TreeContract(const std::vector<Node*>& nodes,
									Node* root, int num_threads)
	{
		root_ready.store(false, std::memory_order_relaxed); // reset

		int n = nodes.size();
		int per_thread = n / num_threads + 1;

		SimplePool threads(num_threads);

		while (!root_ready.load(std::memory_order_acquire)){
			for (int i = 0; i < n; i += per_thread){
				threads.push(schedule_contract, nodes, i,
							 std::min(i + per_thread, n),
							 std::ref(threads), root);
			}

			std::unique_lock<std::mutex> lk(root_mx);
			root_cv.wait_for(lk, std::chrono::milliseconds(1),
							 [] { return root_ready.load(std::memory_order_acquire); });
		}

		threads.stop();
		return 0;
	}
};

#endif
