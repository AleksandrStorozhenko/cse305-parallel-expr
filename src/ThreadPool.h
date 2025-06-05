#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>

template<class E>
class SafeUnboundedQueue {
        std::queue<E> elements;
        std::mutex lock;
        std::condition_variable not_empty;
        std::condition_variable empty;
    public:
        void push(const E& e) {
            std::lock_guard<std::mutex> lk(lock);
            bool wasEmpty = elements.empty();
            elements.push(e);
            if (wasEmpty) not_empty.notify_all();
        }

        E pop() {
            std::unique_lock<std::mutex> lk(lock);
            while (elements.empty()) {
                empty.notify_all();
                not_empty.wait(lk);
            }
            E e = std::move(elements.front());
            elements.pop();
            if (elements.empty()) empty.notify_all();
            return e;
        }

        void waitEmpty() {

            std::unique_lock<std::mutex> lk(lock);
            while (!elements.empty()){
                std::cout<<"Waiting more for empty. size is = "<<elements.size()<<std::endl;
                 empty.wait(lk);
            }
        }
};

class SimplePool {
        unsigned num_workers;
        std::vector<std::thread> workers;
        SafeUnboundedQueue<std::function<bool()>> tasks;

        void do_work() {
            bool cont = true;
            while (cont) cont = tasks.pop()();
        }

    public:
        explicit SimplePool(unsigned n = 0) : num_workers(n) {
            for (unsigned i = 0; i < num_workers; ++i)
                workers.emplace_back(&SimplePool::do_work, this);
        }

        ~SimplePool() { stop(); }

        template<class F, class... Args>
        void push(F f, Args... args) {
            tasks.push([f, args...]() -> bool { f(args...); return true; });
        }

        void waitEmpty() { tasks.waitEmpty(); }

        void stop() {
            // don't wait. assume when calling stop we just need to kill the threadPool
            for (unsigned i = 0; i < num_workers; ++i)
                tasks.push([] { return false; });
            for (unsigned i = 0; i < num_workers; ++i)
                if (workers[i].joinable()) workers[i].join();
        }
};

#endif

