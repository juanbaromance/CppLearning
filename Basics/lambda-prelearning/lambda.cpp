#include <iostream>
#include <algorithm>

template <class T>
void auditor(std::string backtrace, T arg)
{
    (std::cout << backtrace << " : " << arg << std::endl).flush();
}
template <typename Functor>
void f(Functor functor)
{
    std::cout << __PRETTY_FUNCTION__ << functor() << " : " << std::endl;
}

/* Or alternatively you can use this
    void f(std::function<int(int)> functor)
    {
     std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
    */

int g() { static int i = 0; return i++; }



int main()
{
    auto k = 300;
    
    ( [&]() { auditor( __PRETTY_FUNCTION__ , ++k ); } )();
    auditor( __PRETTY_FUNCTION__ , k );

    {   
        auto i = 10;
        auto modulate = [=,&k]( int m = 200 )
        { 
            std::string tmp;
            for( auto o : { i, ++k, m } )
                tmp += std::to_string( o ) + " ";
            return tmp;
        };

        ( [=]() { auditor( __PRETTY_FUNCTION__ , modulate(300) ); } )();
    }
    
    {
        int x = 0, j = 1;
        auditor( __PRETTY_FUNCTION__ + std::to_string(x),[x]() mutable { return ++x; } () );
    }

    {
        auto lambda_func = [i = 0]() mutable { return i++; };
        f(lambda_func); // Pass lambda
        f(g);  // Pass function
    }


    return 0;
}
