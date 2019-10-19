#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <chrono>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <valarray>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <ctime>
#include <sys/time.h>

using namespace boost;
using boost::format;

using namespace std;


static string snapshot()
{
    std::ostringstream oss;
    oss << " ( " << setw(5) << getpid( ) << " )( ";
    time_t t; time( & t );
    struct tm * tm = localtime( & t );
    struct timeval tv; gettimeofday( & tv, nullptr);
    oss << setfill ('0') << setw (2) << tm->tm_hour;
    oss << ":" << setfill ('0') << setw (2) << tm->tm_min;
    oss << ":" << setfill ('0') << setw (2) << tm->tm_sec;
    oss << ":" << setfill ('0') << setw (3) << static_cast<int>(round( tv.tv_usec / 1000 )) << " ) ";
    return oss.str( );
}

template <typename ...T>
void auditor( std::string backtrace, T... args )
{
    cout << boost::format("%s: %20s\n") % snapshot() % backtrace;
}


class MyThread {
public:
    MyThread()
    {
        thread( & MyThread::alternative, this ).join();
    }

private:
    void alternative()
    {
        PointersStuff();

        return;

        VectorDangling();
        regex_sampler( "channel23");
        this_thread::sleep_for( std::chrono::milliseconds(100));
        cout << boost::format("%s : %20s\n") % snapshot() %  __PRETTY_FUNCTION__ ;
        valArrayTesting();
        lambdaTesting0();
        lambdaTesting1();
    }



    void PointersStuff()
    {
        static auto sleep = [](int elapsed){ this_thread::sleep_for( std::chrono::milliseconds(elapsed)); };

        class House
        {
        public:
            House( string _name ="" ) : name( _name ) {}
            ~House(){ auditor( __FUNCTION__ + ( name.empty() ? "" : ( string( "." )+ name ) ) );  }
            const string & named(){ return name; }

        private:
            string name;
        };      


        {
            cout << std::endl;
            auditor("UniquePointer and Move rationale:" );

            using HousePtr = std::unique_ptr<House>;
            class HouseKeeper
            {
            public:
                HouseKeeper( HousePtr house_owner = nullptr )
                {
                    if( house_owner != nullptr )
                        owner = std::move( house_owner );
                }
                void slide( HousePtr house_owner ){ owner = std::move( house_owner ); }
                ~HouseKeeper(){ auditor( __FUNCTION__ ); }

            private:
                HousePtr owner;
            };

            using HouseKeeperPtr = std::unique_ptr<HouseKeeper>;
            HouseKeeperPtr keeper( new HouseKeeper() );
            {
                HousePtr p( new House( "DieFirst"));
                keeper->slide( std::make_unique<House>("DieJoined") );
            }
        }


        {
            cout << std::endl;
            auditor("SharedPointer rationale:" );
            using HousePtr = std::shared_ptr<House>;

            HousePtr h( new House("Shared House") );
            thread( []( HousePtr h ){ auditor( h->named() ); }, h ).detach();
            thread( []( HousePtr h ){ sleep(1000); auditor( h->named() ); }, h  ).detach();
        }

        sleep(5000);


    }

    void VectorDangling()
    {
        cout <<  __PRETTY_FUNCTION__ << "Testing" << endl;
        vector<int> v;

        v.push_back(5);
        auto m = v.begin();
        auto i = & v[ 0 ];
        cout << i << " maps :" << i[0] << endl;
        v.push_back(7);

        /* Now the quiz starts ! */
        cout << i << " maps :" << i[0] << endl;
        for( size_t j = 0; j < v.size(); j++ )
            cout << "index " << j << "("  << & v[j] << ")" << " maps " << v[j] << endl;
        cout << "Begin maps :: " << m[0] << endl;
    }

    constexpr bool test(){ return true; }




    void regex_sampler( string s )
    {
        string channel("invalid string");
        cmatch what;
        if (regex_match(s.c_str(), what, regex("channel(\\d+)")))
            if (what[1].matched)
                channel = what[1].first;
        cout << boost::format("%s : %20s(%s) : %s\n") % snapshot() % __PRETTY_FUNCTION__ % s % channel;
    }


    void valArrayTesting()
    {
        float t[] = { 1.18, 1.30, 1.23, 1.24 };
        float k[] = { 1.0, 1.0, 1.0, 1.0 };
        float offset = 0;

        cout << boost::format("%s : %s\n") % snapshot() % __PRETTY_FUNCTION__ ;

        for( const auto i : t )
            cout << i << endl;

        valarray<float> tmp(t,4),x(k,tmp.size());
        cout << boost::format("%s : %s %4.0f")
                % snapshot() % __PRETTY_FUNCTION__ %  ( inner_product(t,t+4,k,offset)/tmp.size() ) << endl;
        cout << "avg(" << tmp.sum()/tmp.size() << ")" << "max,min(" << tmp.min() << "," << tmp.max() << ")" << endl;
        cout << ( tmp * x ).sum()/tmp.size() << endl;
        tmp = tmp.apply([](float i){ return i +1; });
    }


    void lambdaTesting0()
    {
        cout << boost::format("%s : %s") % snapshot() % __PRETTY_FUNCTION__  ;

        vector<int> v = { 1, 2, 3, 4};
        auto show=[v]()
        {
            for (const auto i : v )
                cout << i << " ";
        };

        float plusone = 2.0;
        auto f = [&plusone](float i) -> float
        {
            return plusone = i +1 + plusone;
        };

        rotate(v.rbegin(),v.rbegin()+1, v.rend());
        show();
        cout << endl << f(0) << ":" << plusone << endl;

    }

    void lambdaTesting1()
    {
        enum {

            WakeUp    = 0,
            Configure = 1,
            Zeroed    = 2,
            Signature = 3,
        };


        bitset<8> tmp(0);
        auto v = { WakeUp, Configure, Zeroed, Signature };
        for_each( v.begin(), v.end(), [ & tmp ](auto i) mutable { tmp.set(i); } );
        cout << boost::format("%s : %s") % snapshot() % __PRETTY_FUNCTION__ << tmp << endl;

    }

};

/* Variadic templates stuff */
template <template <typename, typename> class ContainerType, typename ValueType, typename AllocType>
void print_container(const ContainerType<ValueType, AllocType>& c)
{
    cout << __PRETTY_FUNCTION__ << endl;
    for ( const auto& v : c )
        cout << v << ' ';
    cout << '\n';
}


template <class I, class C, typename...Args >
class cSingleton
{
public:
    static I* Instance( Args... args )
    {
        static I* tmp = nullptr;
        return ( tmp = ( ( tmp == 0 ) ? new C(args...) : tmp ) );
    }
};

/* Template to generate interfaces */
template<class T, class T2 = int>
class iA {
public :
    static iA* instance( T2 value = -2000 ){ return cSingleton<iA,T,T2>::Instance(value);}
public:
    virtual void f() = 0;
    virtual void lambda_testing( vector<T2> v ) = 0;
};


/* Template to generate concrete stuff */
template <typename B=int>
class tA : public iA<tA<B>,B> {
public:
    tA( B value ) : _value( value ){}
private:
    void f() { cout << __PRETTY_FUNCTION__ << to_string( _value ) << '\n'; }
    void lambda_testing( vector<B> v )
    {
        B tmp = 0;
        // auto lambda = [](vector<B> v, B& acc){ acc = accumulate(v.begin(),v.end(),acc); };
        // lambda(v,tmp);
        thread( [](vector<B> v, B& acc){
            acc = accumulate(v.begin(),v.end(),acc);
        },
        v, std::ref(tmp) ).join();
        _value = tmp;
        f();
    }
    B _value;
};



int main()
{
    MyThread();
    return 0;

    print_container( list<int>( {1, 3, 4000 } ) );
    print_container( vector<float>( {1.2, 3, 4000 } ) );
    
    vector<size_t> v = { 1, 2, 3 };
    for( const auto i : v )
        size_t j = i;

    /* Variadic testing */
    //    auto a = iA<tA<>>::instance(3000);
    //    a->f();
    auto aa = iA<tA<float>,float>::instance( -3.2 );
    aa->lambda_testing( {1,2,3,4,5} );
    aa->f();
    return 0;
}
