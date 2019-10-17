#include "futures.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <optional>

using QImpl = MTQ<int>;
using QIface = std::unique_ptr<iMTQ<int,QImpl>>;

template <class ...T>
void auditor(std::string backtrace, T... arg)
{
    (std::cout << backtrace << std::endl).flush();
}

static void testQImpl( QIface&& q)
{
    for( auto k : { 0, 1, 2 } )
        q->push(k);
    std::optional<int> k;
    while( ( k = q->pop( 10 ) ).has_value() )
        auditor( __PRETTY_FUNCTION__  + std::to_string( k.value() ) );
}

int main()
{

    testQImpl( QIface( new QImpl(5) ) );

    auto pool = std::make_shared<thread_pool>();
    pool->add([]() { std::cout << "Hi from thread pool\n"; });
    auto f = async(pool, [](){ std::cout << "Hello\n"; return 1; });
    auto f2 = f.then([](auto& f) { std::cout << f.get() << " World\n"; return 2.0; });
    f2.get();
}
