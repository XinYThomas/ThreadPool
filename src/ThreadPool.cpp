//
// Created by XinY on 25-1-15.
//
#include "ThreadPool.h"

ThreadPool &ThreadPool::getInstance(size_t threads) {
    static ThreadPool instance(threads);
    return instance;
}

ThreadPool::ThreadPool(size_t threads)
    :stop(false)
{
    for(size_t i = 0; i < threads; i++)
    {
        workers.emplace_back(
                [this] {
                    for(;;) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,
                                                 [this] {
                                                     return this->stop || !this->tasks.empty();
                                                 });
                            if(this->stop && this->tasks.empty()) {
                                return;
                            }

                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                }
                );
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers) {
        worker.join();
    }
}

//template<class F, class... Args>
//auto ThreadPool::enqueue(F &&f, Args &&...args) -> std::future<typename std::invoke_result<F(Args...)>::type> {
//    // 返回类型
//    using return_type = typename std::invoke_result<F, Args...>::type;
//    // 获取智能指针
//    auto task = std::make_shared<std::packaged_task<return_type()>>(
//            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
//    );
//    // 等待任务类型返回
//    std::future<return_type> res = task->get_future();
//    {
//        std::unique_lock<std::mutex> lock(queue_mutex);
//        if(stop) {
//            throw std::runtime_error("enqueue on stopped ThreadPool");
//        }
//        tasks.emplace([task](){ (*task)(); });
//    }
//    condition.notify_one();
//    return res;
//}
