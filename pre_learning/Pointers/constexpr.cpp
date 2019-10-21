#include <valarray>

template <typename T=int>
// constexpr 
T probe()
{
    auto acc = 0;

    // int v[] = { 1,2,3 };
    // for ( int i = 0; i < 3; i++ )
    //   acc += v[ i ];
    for( auto i : { 1, 2, 3 } )
        acc += i;
    return acc;
}

int main()
{
    return probe();
}


