//
// Created by XinY on 25-1-15.
//

#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

/**
 * @brief 线程池类，管理一组工作线程
 *
 * 这个类实现了一个线程池，它维护一组工作线程和一个任务队列。
 * 使用单例模式确保整个程序只有一个线程池实例。
 */
class ThreadPool {
public:
    //std::thread::hardware_concurrency(): 用于获取系统支持的并发线程数。
    /**
     * @brief 获取线程池的单例实例
     *
     * @param threads 线程池中线程的数量，默认使用硬件支持的并发线程数
     * @return ThreadPool& 返回线程池单例的引用
     */
    static ThreadPool& getInstance(size_t threads = std::thread::hardware_concurrency());

    // 禁用拷贝构造函数和赋值运算符
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief 向线程池提交任务
     *
     * @tparam F 函数类型
     * @tparam Args 函数参数类型包
     * @param f 要执行的函数
     * @param args 函数的参数
     * @return std::future<typename std::result_of<F(Args...)>::type>
     *         返回一个future对象，用于获取任务的执行结果
     * @throws std::runtime_error 如果线程池已经停止，则抛出异常
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
    {
        // 返回类型
        using return_type = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

        // 获取智能指针
        auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        // 等待任务类型返回
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if(stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks.emplace([task](){ (*task)(); });
        }
        condition.notify_one();
        return res;
    }


private:
    /**
     * @brief 构造函数，创建线程池
     *
     * @param threads 要创建的工作线程数量
     */
     explicit ThreadPool(size_t threads);

    /**
     * @brief 析构函数，确保所有线程正确退出
     */
    ~ThreadPool();

    std::vector<std::thread> workers; ///< 工作线程容器
    std::queue<std::function<void()>> tasks; ///< 任务队列

    std::mutex queue_mutex; ///< 用于同步的互斥量
    std::condition_variable condition; ///< 用于线程通信的条件变量
    bool stop; ///< 线程池停止标志
};

#endif //THREADPOOL_THREADPOOL_H
