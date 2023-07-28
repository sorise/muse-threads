#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "thread_pool/pool.hpp"
#include "thread_pool/concurrent_pool.hpp"

using namespace muse::pool;
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

ThreadPool pool(4,8,1024, ThreadPoolType::Flexible, ThreadPoolCloseStrategy::WaitAllTaskFinish, 2500ms);

TEST_CASE("normal - commit", "[ThreadPool]"){
    auto ex = make_executor([](int i)->int{
        return i * i;
    }, 10);

    if(pool.commit_executor(ex).isSuccess){
        int result = ex->get();
        REQUIRE(result == 100);
    }
}

TEST_CASE("normal - reference", "[ThreadPool]") {
    Normal normal(50, "remix");

    auto ex = make_executor(&Normal::setValueAndGetName, normal, 100);

    pool.commit_executor(ex);

    ex->get();

    std::cout << normal.getValue() << std::endl;
}

TEST_CASE("normal - run", "[ConcurrentThreadPool]"){
    muse::pool::ConcurrentThreadPool cpool(4, 1024,ThreadPoolCloseStrategy::WaitAllTaskFinish, 1500ms);

    int isSuccess = true;
    try {
        for (int i = 0; i < 1200; ) {
            auto token1 = make_executor([&](int i)->void{
                std::this_thread::sleep_for( std::chrono::milliseconds(5));
                printf("logger: %d is running!\n", i);
            }, i);
            auto result = cpool.commit_executor(token1);
            if (!result.isSuccess){
                std::this_thread::sleep_for( std::chrono::milliseconds(5));
            }else{
                i++;
            }
        }

        std::this_thread::sleep_for(4000ms);

        auto token1 = make_executor([](int i)->void{
            std::this_thread::sleep_for( std::chrono::milliseconds(5));
            printf("logger: %d is running!\n", i);
        }, 4000);

        cpool.commit_executor(token1);

        std::this_thread::sleep_for(1600ms);

        for (int i = 4001; i < 4501; ) {
            auto newToken = make_executor([&](int i)->void{
                std::this_thread::sleep_for( std::chrono::milliseconds(5));
                printf("logger: %d is running!\n", i);
            }, i);
            auto result = cpool.commit_executor(newToken);
            if (!result.isSuccess){
                std::this_thread::sleep_for( std::chrono::milliseconds(5));
            }else{
                i++;
            }
        }
    }catch(...){
        isSuccess = false;
    }

    cpool.close();
    REQUIRE(isSuccess);
}