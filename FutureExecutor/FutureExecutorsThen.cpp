#include "futures.hpp"
#include <iostream>
#include <memory>

int main()
{
    MTQTesting::Testing<int>().Async();
    return 0;

    auto pool = std::make_shared<thread_pool>();
    pool->add([]() { std::cout << "Hi from thread pool\n"; });
    auto f = async(pool, []()
                   {
                       std::cout << "Hello\n"; return 1;
                   }
                   );
    auto f2 = f.then([](auto& f)
                     {
                         std::cout << f.get() << " World\n";
                         return 2.0;
                     });
    f2.get();
}
