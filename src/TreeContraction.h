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
                threads.push([n = nodes[i]](){ n->contract(); });
            }
        }
    }

    static std::size_t TreeContract(const std::vector<Node*>& nodes, Node* root, int num_threads)
    {
        std::cout<<"Starting treeContract with "<<nodes.size()<<" and threads = "<<num_threads<<std::endl;
        int n = nodes.size();
        int per_thread = n / num_threads + 1;

        SimplePool threads(num_threads);
        int count = 0;
        while(!root->isDone() && count < 5){
            std::cout<<"Finished = "<<root->value.has_value()<<std::endl;
            for(int i = 0; i < n; i+= per_thread){
                std::cout<<"Scheduling from"<<i<<" to "<<std::min(i + per_thread, n)<<std::endl;
                threads.push(schedule_contract, nodes, i, std::min(i + per_thread, n), std::ref(threads));
            }
            usleep(10000);
            count++;
        }
        threads.stop();
        return 0;
    }

};

#endif
