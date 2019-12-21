#pragma once

#include <functional>
#include <future>
// #include <mutex>
// #include <thread>
#include <utility>
#include <vector>
#include <pthread.h>
#include "ConcurrentQueue.h"

using namespace std;

static void * threadLoop(void *arg);

class ThreadPool {
private:
    class ThreadWorker {
    private:
        int m_id;
        ThreadPool *m_pool;
    public:
        ThreadWorker(ThreadPool *pool, const int id)
                : m_pool(pool), m_id(id) {
        }

        void operator()() {
            function<void()> func;
            bool dequeued;
            while (!m_pool->m_shutdown) {
                {
                    pthread_mutex_lock(&(m_pool->m_conditional_mutex));
                    // unique_lock<mutex> lock(m_pool->m_conditional_mutex);
                    if (m_pool->m_queue.empty()) {
                        pthread_cond_wait(&(m_pool->m_conditional_lock), &(m_pool->m_conditional_mutex));
                        // m_pool->m_conditional_lock.wait(lock);
                    }
                    dequeued = m_pool->m_queue.dequeue(func);
                }
                if (dequeued) {
                    func();
                }
            }
        }
    };

    bool m_shutdown;
    ConcurrentQueue<function<void()>> m_queue;
    // vector<thread> m_threads;
    vector<pthread_t> m_threads;
    // mutex m_conditional_mutex;
    pthread_mutex_t m_conditional_mutex;
//    pthread_mutex_t ThreadPool::m_conditional_mutex = PTHREAD_MUTEX_INITIALIZER;
//    condition_variable m_conditional_lock;// = PTHREAD_MUTEX_INITIALIZER
    pthread_cond_t m_conditional_lock;
    pthread_attr_t attr;
    //PTHREAD_COND_INITIALIZER

public:
    ThreadPool(const int n_threads)
            // : m_threads(vector<thread>(n_threads)), m_shutdown(false) {
            : m_threads(vector<pthread_t>(n_threads)), m_shutdown(false) {
        pthread_cond_init(&m_conditional_lock, nullptr);
        pthread_mutex_init(&m_conditional_mutex, nullptr);
        pthread_attr_init(&attr);
    }

    void init() {
        for (int i = 0; i < m_threads.size(); ++i) {
            ThreadWorker threadWorker = *(ThreadWorker*)calloc(1, sizeof(ThreadWorker));

            pthread_create(&m_threads[i], &attr, ThreadWorker(this, i), i);
//             m_threads[i] = thread(ThreadWorker(this, i));
        }
    }

    void shutdown() {
        m_shutdown = true;
        // m_conditional_lock.notify_all();
        pthread_mutex_lock(&m_conditional_mutex);
        pthread_cond_broadcast(&m_conditional_lock);
        pthread_mutex_unlock(&m_conditional_mutex);

        for (auto &m_thread : m_threads) {
            if (m_thread.joinable()) {
                pthread_join(&, &m_thread);
            }
        }
    }

    template<typename F, typename...Args>
    auto add_task(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
        // Create a function with bounded parameters ready to execute
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        // Encapsulate it into a shared ptr in order to be able to copy construct / assign
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Wrap packaged task into void function
        std::function<void()> wrapper_func = [task_ptr]() {
            (*task_ptr)();
        };

        // Enqueue generic wrapper function
        m_queue.enqueue(wrapper_func);

        // Wake up one thread if its waiting
        // m_conditional_lock.notify_one();
        pthread_mutex_lock(&m_conditional_mutex);
        pthread_cond_signal(&m_conditional_lock);
        pthread_mutex_unlock(&m_conditional_mutex);

        // Return future from promise
        return task_ptr->get_future();
    }
};

void * ThreadPool:threadLoop(void *arg) {

};
