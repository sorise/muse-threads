#include <iostream>
#include <random>
#include <ctime>
#include <exception>
#include <chrono>
#include "thread_pool/pool.hpp"
#include "thread_pool/concurrent_pool.hpp"

using namespace muse::pool;

int main() {
    //创建一个线程池
    //队列最大程度为 1024
    //线程数量可以动态变化，最多线程数量为 8，最少为 4
    //线程关闭策略为 等待所有任务执行完毕后才关闭
    //线程池 管理线程运行频率为 1.9s
    ThreadPool<ThreadPoolType::Flexible, 1024, 8> pool(4, ThreadPoolCloseStrategy::WaitAllTaskFinish, 1900ms);

    //将任务包装成一个执行器
    auto executor = make_executor([](int i)->int{
        std::this_thread::sleep_for( std::chrono::milliseconds(5));
        return i * i;
    }, 4000);
    //提交执行器
    auto commitResult = pool.commit_executor(executor);
    //如果提交成功
    if (commitResult.isSuccess){
        //执行过程中没有异常
        if (!executor->isError()){
            //获取结果
            std::cout << "result :" << executor->get() << std::endl;
        }
    }
    //关闭线程池
    pool.close();
    return 0;
}
