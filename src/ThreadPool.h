#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>
#include <unistd.h>

template<class E>
class SafeUnboundedQueue {
    private:
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
            while (!elements.empty()) empty.wait(lk);
        }
        bool isEmpty() const { return elements.empty(); }
};

class SimplePool {
        unsigned num_workers;
        std::vector<std::thread> workers;
        SafeUnboundedQueue<std::function<bool()>> tasks;

        // track tasks that have been popped but not finished
        std::atomic<std::size_t> active{0};
        std::mutex idle_mtx;
        std::condition_variable idle_cv;
        
        void do_work() {
            bool cont = true;
            while (cont) {
                auto task = tasks.pop();
                active.fetch_add(1);
                cont = task();
                if (active.fetch_sub(1) == 1) {
                    std::lock_guard<std::mutex> lk(idle_mtx);
                    idle_cv.notify_all();
                }
            }
        }

    public:
        std::mutex owner;
        
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

        // queue empty and no active tasks
        void waitIdle() {
            std::unique_lock<std::mutex> lk(idle_mtx);
            while(!(active.load() == 0 && tasks.isEmpty())){
                tasks.waitEmpty();
                idle_cv.wait(lk);
            }
        }

        void stop() {
            waitIdle(); // better shutdown
            for (unsigned i = 0; i < num_workers; ++i)
                tasks.push([] { return false; });
            for (auto& w : workers)
                if (w.joinable()) w.join();
        }
};

#endif

