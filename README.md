# [muse-threads](#)
一个基于C++ 11、支持任务批量提交、header-only的线程池，提供了基于std::queue和无锁队列 [concurrentqueue](https://github.com/cameron314/concurrentqueue)的两个线程池版本。
* ThreadPool 基于C++ STL 标准容器 queue。
* ConcurrentThreadPool 基于无锁队列 concurrentqueue。

**ThreadPool 线程池设计图：**  
<img src="./docs/assets/jiagou.png" width="800px" />

**特点**:
* 支持批量任务提交
* 使用简单、跨平台、全部基于 C++ 标准。
* 支持锁队列 - `在CPU几乎满负荷的情况下推荐使用！`
* 支持配置

#### 单个任务提交：
```cpp
//创建一个线程池
//队列最大程度为 1024
//线程数量可以动态变化，最多线程数量为 8，最少为 4
//线程关闭策略为 等待所有任务执行完毕后才关闭
//线程池 管理线程运行频率为 1.9s
ThreadPool<ThreadPoolType::Flexible, 1024, 8> 
        pool(4, ThreadPoolCloseStrategy::WaitAllTaskFinish, 1900ms);

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
```

#### 批量任务提交：

```cpp
int main() {
    //创建一个线程池
    //队列最大长度为 1024
    //线程数量可以动态变化，最多线程数量为 8，最少为 4
    //线程关闭策略为 等待所有任务执行完毕后才关闭
    //线程池 管理线程运行频率为 1.9s
    ThreadPool<ThreadPoolType::Flexible, 1024, 8> 
            pool(4, ThreadPoolCloseStrategy::WaitAllTaskFinish, 1900ms);

    std::vector<std::shared_ptr<Executor>> executors;

    for (int i = 0; i < 10; ++i) {
        auto executor = make_executor([](int i)->int{
            std::this_thread::sleep_for( std::chrono::milliseconds(5));
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
        }
    }

    //关闭线程池
    pool.close();
    return 0;
}
```
