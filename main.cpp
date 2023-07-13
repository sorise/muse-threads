#include <iostream>
#include <random>
#include <ctime>
#include <exception>
#include <chrono>
#include "thread_pool/pool.hpp"
#include "thread_pool/executor_token.hpp"
#include "thread_pool/concurrent_pool.hpp"

using namespace muse::pool;

int main() {
    //创建一个线程池
    //队列最大长度为 1024
    //线程数量可以动态变化，最多线程数量为 8，最少为 4
    //线程关闭策略为 等待所有任务执行完毕后才关闭
    //线程池 管理线程运行频率为 1.9s
    ThreadPool<ThreadPoolType::Fixed, 1024, 4> pool(4, ThreadPoolCloseStrategy::WaitAllTaskFinish, 1900ms);

    std::vector<std::shared_ptr<Executor>> executors;

    for (int i = 0; i < 10; ++i) {
        auto executor = make_executor([](int i)->int{
            printf("logger run %d !\n", i);
            return i * i;
        }, i);
        executors.push_back(executor);
    }

    //将任务包装成一个执行器

    //提交执行器
    std::vector<CommitResult> commitResults = pool.commit_executors(executors);

    for (int i = 0; i < commitResults.size(); ++i) {
        //如果提交成功
        if (commitResults[i].isSuccess){
            //执行过程中没有异常
            if (!is_error_executor<int>(executors[i])){
                //获得结果
                int value = get_result_executor<int>(executors[i]);
                printf("executor[%d] - result: %d\n", i, value);
            }
        }else{
            printf("task[%d] submit failed\n", i);
        }
    }

    //关闭线程池
    pool.close();
    return 0;
}

