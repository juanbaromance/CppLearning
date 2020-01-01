#include <valarray>
#include <tr1/functional>
#include <iostream>

template <typename T>
// constexpr 
T probe()
{

    T acc = 0;

    int v[] = { 1,2,3 };
    for ( int i = 0; i < 3; i++ )
       acc += v[ i ];

//    for( auto i : { 1, 2, 3 } )
//        acc += i;

    return acc;
}

const int& myFun( int m )
{
    static const int p = m;
    return p;
}

class MyClass {
public:
    MyClass( const int & offset ) : p(offset){}
    const int& myFun( int ){
        return p;
    }
private:
    const int p;
};

int main()
{
    std::tr1::function<const int&()> F = std::tr1::bind(myFun,30);
    std::cout << __PRETTY_FUNCTION__ << ":" << F() << "\n";

    MyClass c(34);
    F = std::tr1::bind( & MyClass::myFun, & c, 32 );
    if( F )
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << F() << "\n";
        std::abort();
    }

    int p = probe<int>();
    return p;
}


