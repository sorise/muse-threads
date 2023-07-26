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


TEST_CASE("make_executor - number", "[ExecutorToken]"){

    Normal normal(25,"remix");

    REQUIRE_NOTHROW(make_executor(&Normal::setValueAndGetName, normal, 56));
}