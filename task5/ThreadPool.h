#pragma once

#include <functional>
#include <future>
#include <utility>
#include <vector>
#include <pthread.h>
#include "ConcurrentQueue.h"

typedef void *(*THREADFUNC)(void *);

using namespace std;

class ThreadPool {
public:
    /**
     * This class works with thread
     */
    class ThreadWorker {
    private:
        int m_id;
        ThreadPool *m_pool2;

    public:
        /**
         * This is the main function, which performs func()
         * @param data
         * @return
         */
        void *initWorker(void *data) {
            function<void()> func;
            bool dequeued;
            while (!m_pool2->m_shutdown) {
                {
                    pthread_mutex_lock(&(m_pool2->m_conditional_mutex));
                    if (m_pool2->m_queue.empty()) {
                        pthread_cond_wait(&(m_pool2->m_conditional_lock), &(m_pool2->m_conditional_mutex));
                    }
                    dequeued = m_pool2->m_queue.dequeue(func);
                }
                if (dequeued) {
                    func();
                }
                pthread_mutex_unlock(&(m_pool2->m_conditional_mutex));
            }
        }

        ThreadWorker(ThreadPool *pPool, int i) {
            m_pool2 = pPool;
            m_id = i;
        }
    };

    typedef struct T {
        ThreadPool *pool;
        int m_id;
    } T;

    bool m_shutdown; // when thread pool shutdowns (all thread are joined)
    ConcurrentQueue<function<void()>> m_queue;
    vector<pthread_t> m_threads; // *list* of threads
    pthread_mutex_t m_conditional_mutex;
    pthread_cond_t m_conditional_lock;
    pthread_attr_t attr;
public:
    explicit ThreadPool(const int n_threads) // init cond, mutex and attr
            : m_threads(vector<pthread_t>(n_threads)), m_shutdown(false) {
        pthread_cond_init(&m_conditional_lock, nullptr);
        pthread_mutex_init(&m_conditional_mutex, nullptr);
        pthread_attr_init(&attr);
    }

    void init() {
        for (int i = 0; i < m_threads.size(); ++i) {
            ThreadWorker t = ThreadWorker(this, i);

            T *tt = (T *) calloc(1, sizeof(T));
            tt->pool = this;
            tt->m_id = i;

            auto w = new ThreadWorker(this, i);
            pthread_create(&m_threads[i], &attr, (THREADFUNC) &ThreadWorker::initWorker, w); // It's normal (compilation error)
            // run new thread
        }
    }

    void shutdown() {
        m_shutdown = true;
        pthread_mutex_lock(&m_conditional_mutex);
        pthread_cond_broadcast(&m_conditional_lock); // like notifyAll in Java
        pthread_mutex_unlock(&m_conditional_mutex);

        for (auto &m_thread : m_threads) { // join all threads
            pthread_join(m_thread, NULL);
        }
    }

    /**
     * Very smart method (from the Internet exactly). It can simplify, but we can't:)
     * @tparam F
     * @tparam Args
     * @param f
     * @param args
     * @return
     */
    template<typename F, typename...Args>
    auto add_task(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        std::function<void()> wrapper_func = [task_ptr]() {
            (*task_ptr)();
        };

        m_queue.enqueue(wrapper_func);

        pthread_mutex_lock(&m_conditional_mutex);
        pthread_cond_signal(&m_conditional_lock);
        pthread_mutex_unlock(&m_conditional_mutex);

        return task_ptr->get_future();
    }
};