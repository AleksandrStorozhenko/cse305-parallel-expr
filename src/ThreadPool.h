#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t nThreads) {
        nThreads = nThreads ? nThreads : 1;
        for (std::size_t i = 0; i < nThreads; ++i)
            workers_.emplace_back([this]{ worker(); });
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lk(m_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& t : workers_) t.join();
    }

    bool enqueue(const std::function<void()>& job) {
        std::lock_guard<std::mutex> lk(m_);
        if (stop_) return false;
        q_.push(job);
        cv_.notify_one();
        return true;
    }

    void wait_idle() {
        std::unique_lock<std::mutex> lk(m_);
        idleCv_.wait(lk, [this]{ return q_.empty() && active_ == 0; });
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    void worker() {
        for (;;) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lk(m_);
                cv_.wait(lk, [this]{ return stop_ || !q_.empty(); });
                if (stop_ && q_.empty()) return;
                job = std::move(q_.front());
                q_.pop();
                ++active_;
            }
            job();
            {
                std::lock_guard<std::mutex> lk(m_);
                --active_;
                if (q_.empty() && active_ == 0) idleCv_.notify_all();
            }
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> q_;
    std::mutex m_;
    std::condition_variable cv_, idleCv_;
    std::atomic<std::size_t> active_{0};
    bool stop_{false};
};

#endif
