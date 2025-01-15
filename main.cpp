#include "gtest/gtest.h"
#include <future>
#include <iostream>
#include <thread>
#include <chrono>
#include "ThreadPool.h"
/**
 * @brief 基本的future使用示例
 */
void basic_future_example() {
    // 使用async创建异步任务
    std::future<int> future = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 42;
    });

    // 获取结果（会阻塞直到结果可用）
    int result = future.get();
    std::cout << "结果: " << result << std::endl;
}

/**
 * @brief 演示future的状态检查
 */
void future_status_example() {
    auto future = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 42;
    });

    // 检查future状态
    while(true) {
        auto status = future.wait_for(std::chrono::milliseconds(100));
        if (status == std::future_status::ready) {
            std::cout << "结果已就绪！" << std::endl;
            std::cout << "值: " << future.get() << std::endl;
            break;
        } else if (status == std::future_status::timeout) {
            std::cout << "仍在等待结果..." << std::endl;
        } else if (status == std::future_status::deferred) {
            std::cout << "任务被推迟执行" << std::endl;
            break;
        }
    }
}

/**
 * @brief 演示promise和future的配对使用
 */
void promise_future_example() {
    std::promise<int> promise;
    std::future<int> future = promise.get_future();

    // 在另一个线程中设置值
    std::thread worker([&promise] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        promise.set_value(42);
    });

    // 在主线程中等待结果
    std::cout << "等待结果..." << std::endl;
    std::cout << "得到结果: " << future.get() << std::endl;

    worker.join();
}

TEST(test, test)
{
    std::cout << std::thread::hardware_concurrency() << std::endl;
    basic_future_example();
    future_status_example();
    promise_future_example();
}

int fibonacci(int n) {
    if (n < 2) return n;
    return fibonacci(n-1) + fibonacci(n-2);
}

TEST(threadpool, threadpool)
{
    try {
        // 获取线程池实例
        ThreadPool& pool = ThreadPool::getInstance();

        // 存储任务结果的future容器
        std::vector<std::future<int>> results;

        // 提交8个计算斐波那契数列的任务
        for(int i = 0; i < 8; ++i) {
            results.emplace_back(
                    pool.enqueue([i] {
                        return fibonacci(i + 20);
                    })
            );
        }

        // 获取并打印结果
        for(auto& result : results) {
            std::cout << result.get() << ' ';
        }
        std::cout << std::endl;
    }
    catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
