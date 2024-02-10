# [muse-threads](#)
一个基于C++ 11、支持任务批量提交、支持配置的线程池，提供了基于std::queue和无锁队列 [concurrentqueue](https://github.com/cameron314/concurrentqueue)的两个线程池版本。
* ThreadPool 基于C++ STL 标准容器 queue。
* ConcurrentThreadPool 基于无锁队列 [concurrentqueue](https://github.com/cameron314/concurrentqueue)。

**ThreadPool 线程池设计图：**  
<img src="./docs/assets/jiagou.png" width="800px" />

**特点**:
* 支持批量任务提交
* 实现简单、跨平台、全部基于 C++ 标准。
* 支持无锁队列 - `在CPU几乎满负荷的情况下推荐使用！`
* 支持配置

#### 单个任务提交：
单个任务提交十分简单，提高以后会返回一个 CommitResult 对象表示提交是否成功。
```cpp
struct CommitResult{
    bool isSuccess;       //是否添加成功
    RefuseReason reason;  //失败原因
};

enum class RefuseReason{
    NoRefuse,                       //没有拒绝
    CannotExecuteAgain,             //请勿重复执行
    ThreadPoolHasStoppedRunning,    //线程池已经停止运行
    TaskQueueFulled,                //任务队列已经满了
    MemoryNotEnough,                //内存不足
};
```

```cpp
#include "thread_pool/pool.hpp"
#include "thread_pool/executor_token.hpp"

using namespace muse::pool;

int main() {
    //创建一个线程池
    //队列最大程度为 1024
    //线程数量可以动态变化，最多线程数量为 8，最少为 4
    //线程关闭策略为 等待所有任务执行完毕后才关闭
    //线程池 管理线程运行频率为 1.9s
    ThreadPool pool( 4 , 8 , 1024 , 
         ThreadPoolType::Flexible , 
         ThreadPoolCloseStrategy::WaitAllTaskFinish , 
         2500ms
    );
    
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
```
**线程关闭策略**
```c++
/* 线程池关闭策略 */
enum class ThreadPoolCloseStrategy{
    DiscardAllTasks,        //丢弃所有的任务
    WaitAllTaskFinish,      //等待任务完成
    ReturnTaskAndClose,     //返回任务，然后关闭
};
```

#### 批量任务提交：
批量任务的结果或许需要使用一些静态辅助方法(R 为返回值类型，需要指定)！

* is_error_executor\<R\> ：判断结果是否有异常，返回 bool，会阻塞等待方法执行完毕。
* is_finish_executor\<R\> ： 非阻塞方法，判断线程池是否已经执行完毕，返回 bool。
* is_discard_executor\<R\> : 非阻塞方法，判断任务是否已经被线程池丢弃，返回 bool。
* get_result_executor\<R\> ： 获取返回值,阻塞方法。


```cpp
#include "thread_pool/pool.hpp"
#include "thread_pool/executor_token.hpp"

using namespace muse::pool;

int main() {
    //创建一个线程数量固定的线程池
    //队列最大长度为 1024
    //线程关闭策略为 等待所有任务执行完毕后才关闭
    ThreadPool pool( 4 , 4 , 1024 , 
         ThreadPoolType::Fixed , 
         ThreadPoolCloseStrategy::WaitAllTaskFinish
    );
    
    //创建一个批量任务容器
    std::vector<std::shared_ptr<Executor>> executors;
    
    //创建任务
    for (int i = 0; i < 10; ++i) {
        auto executor = make_executor([](int i)->int{
            std::this_thread::sleep_for( std::chrono::milliseconds(5));
            return i * i;
        }, i);
        executors.push_back(executor);
    }

    //提交任务
    std::vector<CommitResult> commitResults = pool.commit_executors(executors);
    
    //获取执行结果
    for (int i = 0; i < executors.size(); ++i) {
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

#### 成员函数提交
支持提交成员函数，可以传递指针或者引用，类似于bind的功能！
```cpp
class Normal{
public:
    Normal(int _value, const std::string& _name)
            :value(_value), name( std::move(_name)){}

    std::string setValueAndGetName(int _new_value){
        this->value = _new_value;
        return this->name;
    }

    int getValue(){
        return this->value;
    }

private:
    int value;
    std::string name;
};

Normal normal(25,"remix");
//将任务包装成一个执行器，传递引用
auto ext = make_executor(&Normal::setValueAndGetName, normal, 56);
/*   传递指针
 *   auto ext = make_executor(&Normal::setValueAndGetName, &normal, 56);
 * */
//提交执行器
pool.commit_executor(ext);

std::cout << ext->get() << std::endl; //remix
std::cout << normal.getValue() << std::endl; //56
```

#### [无所队列线程池](#)
请在服务器几乎满负荷的情况下使用, 除了创建方式之外和普通队列使用几乎一样。 如果线程没有任务执行则处于空闲状态，空闲时间超过设置大小则杀死线程，
等待有新任务的时候再创建线程。
```cpp
/* 创建一个线程池 */
muse::pool::ConcurrentThreadPool cpool(
        4, //最大线程数
        2048,   //队列长度
        ThreadPoolCloseStrategy::WaitAllTaskFinish,//关闭策略 
        1500ms //运行线程空闲时间
);

auto token = make_executor([](int i)->void{
    std::this_thread::sleep_for( std::chrono::milliseconds(5));
    printf("logger: %d is running!\n", i);
}, 4000);

cpool.commit_executor(token);
//关闭
cpool.close();
```