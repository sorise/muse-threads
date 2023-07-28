#include <iostream>
#include <random>
#include <ctime>
#include <exception>
#include <chrono>
#include "thread_pool/pool.hpp"
#include "thread_pool/executor_token.h"

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

using namespace muse::pool;
int main() {

    //创建一个线程池
    ThreadPool pool(
            4, //线程数量可以动态变化，最多线程数量为 8，最少为 4
            8,
            1024,  //队列最大长度为 1024
            ThreadPoolType::Flexible, //动态线程池
            ThreadPoolCloseStrategy::WaitAllTaskFinish, //线程关闭策略为 等待所有任务执行完毕后才关闭
            2500ms //线程池 管理线程运行频率为 2.5s
    );

    Normal normal(50,"remix");

    auto ex = make_executor(&Normal::setValueAndGetName, normal, 100);

    pool.commit_executor(ex);

    ex->get();

    std::cout << normal.getValue() << std::endl;

    //关闭线程池
    pool.close();
    return 0;
}

