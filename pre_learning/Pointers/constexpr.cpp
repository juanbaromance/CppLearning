#include <valarray>

template <typename T=int>
constexpr T probe()
{
    auto acc = 0;
    for( auto i : { 1, 2, 3 } )
        acc += i;
    return acc;
}

int main()
{
    return probe();
}


