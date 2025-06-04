#ifndef TREECONTRACTION_H
#define TREECONTRACTION_H

#include "Node.h"
#include "ThreadPool.h"
#include <vector>
#include <unordered_set>
#include <functional>
#include <unistd.h>

class TreeContraction
{
    
public:
    static void schedule_contract(const std::vector<Node*>& nodes, size_t start, size_t end, SimplePool& threads){
        // std::cout<<"Inside schedule contract "<<std::this_thread::get_id()<<std::endl;

        for(auto i = start; i < end; i++){
            if(!nodes[i]->isDone()){
                // std::cout<<"Node "<<i<<" not done"<<std::endl;
                threads.push([n = nodes[i]](){ n->contract(); });
            }
        }
    }

    static std::size_t TreeContract(const std::vector<Node*>& nodes, Node* root, int num_threads)
    {
        // std::cout<<"Starting treeContract with "<<nodes.size()<<" and threads = "<<num_threads<<std::endl;
        int n = nodes.size();
        int per_thread = n / num_threads + 1;

        SimplePool threads(num_threads);
        while(!root->value.has_value()){
            for(int i = 0; i < n; i+= per_thread){
                // std::log<<"Scheduling from"<<i<<" to "<<std::min(i + per_thread, n)<<std::endl;
                threads.push(schedule_contract, nodes, i, std::min(i + per_thread, n), std::ref(threads));
            }
            // usleep(1);
            threads.waitEmpty();
        }
        threads.stop();
        return 0;
    }

};

#endif
