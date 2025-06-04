#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>

template <class E> 
class SafeUnboundedQueue {
        std::queue<E> elements;
        std::mutex lock;
        std::condition_variable not_empty;
        std::condition_variable empty;
    public: 
        SafeUnboundedQueue<E>(){}
        void push(const E& element);
        E pop ();
        bool is_empty() const {return this->elements.empty();}

        void waitEmpty();
};

template <class E>
void SafeUnboundedQueue<E>::push(const E& element) {
    std::lock_guard<std::mutex> lk(lock);
    bool wasEmpty = is_empty();
    elements.push(element);
    if(wasEmpty){
        not_empty.notify_all();
    }
}

template <class E> 
E SafeUnboundedQueue<E>::pop() {
    std::unique_lock<std::mutex> lk(lock);
    while(is_empty()){
        empty.notify_all();
        not_empty.wait(lk);
    }
    auto elt = elements.front();
    elements.pop();
    return elt;
}

//blocks until empty
template <class E>
void SafeUnboundedQueue<E>::waitEmpty() {
    std::unique_lock<std::mutex> lk(lock);
    while(!is_empty()){
        empty.wait(lk);
    }
    return;
}

//TODO: add timing to pool
class SimplePool {
        unsigned int num_workers;
        std::vector<std::thread> workers;
        SafeUnboundedQueue<std::function<bool()> > tasks;
        
        void do_work();
    public:
        SimplePool(unsigned int num_workers = 0);
        ~SimplePool();
        template <class F, class... Args>
        void push(F f, Args ... args);
        void stop();
        void waitEmpty();
};

void SimplePool::do_work() {
    bool should_continue = true;
    while(should_continue){
        auto task = tasks.pop();
        should_continue = task();
    }
}

SimplePool::SimplePool(unsigned int num_workers) {
    this->num_workers = num_workers;
    for(int i = 0; i < num_workers; i++){
        workers.emplace_back(&SimplePool::do_work, this);
    }
}

SimplePool::~SimplePool() {
    stop();
}

template <class F, class... Args>
void SimplePool::push(F f, Args ... args) {
    tasks.push([f, args...]() -> bool {
        f(args...);
        return true;
    });
}

//blocks until empty
void SimplePool::waitEmpty() {
    tasks.waitEmpty();
}

void SimplePool::stop() {
    // wait until every task is gone
    tasks.waitEmpty();
        
    for(int i = 0; i < num_workers; ++i){
        tasks.push([]() -> bool { return false; });
    }

    for(int i = 0; i < num_workers; ++i){
        if(workers[i].joinable())
            workers[i].join();
    }
}

#endif
