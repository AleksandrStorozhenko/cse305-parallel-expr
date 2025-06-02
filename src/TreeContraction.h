#ifndef TREECONTRACTION_H
#define TREECONTRACTION_H

#include "Node.h"
#include "ThreadPool.h"
#include <vector>
#include <unordered_set>
#include <functional>

class TreeContraction
{
    
public:
    static void schedule_contract(const std::vector<Node*>& nodes, size_t start, size_t end, SimplePool& threads){
        for(auto i = start; i < end; i++){
            if(!nodes[i]->value.has_value()){
                threads.push([n = nodes[i]](){ n->contract(); });
            }
        }
    }

    static std::size_t TreeContract(const std::vector<Node*>& nodes, Node* root, int num_threads)
    {
        int n = nodes.size();
        int per_thread = n / num_threads;
        SimplePool threads(num_threads);
        while(!root->value.has_value()){
            for(int i = 0; i < n; i+= per_thread){

                threads.push(schedule_contract, nodes, i, std::min(i + per_thread, n), std::ref(threads));
            }
        }
        return 0;
    }

};

#endif
