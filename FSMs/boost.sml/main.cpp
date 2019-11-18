#include <iostream>
#include <sys/time.h>
#include <ctime>

#include "boost/sml.hpp"
#include "callables.h"
#include <string>
#include <memory>
#include <cmath>
#include <typeinfo>

template <typename T>
struct crtp {
    T& underly(){ return static_cast<T&>(*this); }
    T const& underly() const { return static_cast<T const&>(*this); }
};
using namespace std;

#include <fmt/format.h>

constexpr static char magics[] = "12345678910";
auto call = []( auto... args ) { return call_impl(args...); };



#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
class cHelper {
public:
    void dump( const std::string & s )
    {
        time_t t; time( & t );
        struct tm * tm = localtime( & t );
        struct timeval tv;
        gettimeofday( & tv, nullptr );
        std::ostringstream oss;
        oss << "( " << setfill ('0') << setw (2) << tm->tm_hour;
        oss << ":" << setfill ('0') << setw (2) << tm->tm_min;
        oss << ":" << setfill ('0') << setw (2) << tm->tm_sec;
        oss << ":" << setfill ('0') << setw (3) << static_cast<int>(round( tv.tv_usec / 1000 )) << " ) ";
        fmt::print( oss.str() + s + "\n");
    }
};

struct sClosed {
    sClosed( shared_ptr<cHelper> helper_ ) : helper( helper_ ){}
    void onEntry(){}
    void onExit(){}
    shared_ptr<cHelper> helper;
};

struct sOpened {

};
struct sLocked {
    sLocked( const string & keyword ) : keyword_ ( keyword ){}
    const string keyword(){ return keyword_ ; }
    string keyword_ ;
};

struct eOpen {};
struct eClose {};
struct eLock
{
private:
    std::string _keyword;
public:
    eLock( const std::string & keyword ) : _keyword( keyword ) {}
    std::string valueOfKeyword(){ return _keyword; }
};
struct eUnLock {};

namespace sml = boost::sml;

static std::string valueOfKeyWord;
static auto  LockGuard   = []( const std::string & keyword ){ return keyword.length() > 10; };
static auto  UnLockGuard = []( const std::string & keyword )
{
    if( keyword == valueOfKeyWord )
        return true;
    fmt::print("\"UNLOCKING\" Action rejected: Key {0:x} doesn't match\n", std::hash<std::string>{}(keyword) );
    return false;
};


struct cDoorLogics {

    auto operator()() noexcept {
        using namespace sml;
        return make_transition_table(

        // StartUp
        *"idle"_s / [&]{ startUp("StartUp sampled"); } = "sClosed"_s

        // Close specification
        , "sClosed"_s + sml::on_entry<_> / [&] { closeEntry(); }
        , "sClosed"_s + sml::on_exit<_>  / [&] { closeExit();  }
        , "sClosed"_s + event<eOpen>     / [&] { dl->dump("Open Transition"); }  = "sOpened"_s

        , "sLocked"_s + event<eOpen>     / ( [&] { dl->dump("\"UNLOCK\" door before OPENING"); } ) = "sLocked"_s
        , "sOpened"_s + event<eClose>    / ( [] { }  )= "sClose"_s

//         , "sOpened"_s + event<eLock>  / ( [&] {dl->dump("\"CLOSE\" door before \"LOCKING\"\n"); } ) = "sOpenened"_s
//         , "sClosed"_s + event<eLock>   [ LockGuard ] / ([]{}) = "sLocked"_s
//         , "sLocked"_s + event<eUnLock> [ UnLockGuard ] / ([]{}) = "sClose"_s
                //         , "s3"_s + event<e4> / [] (const auto& e) { assert(42 == e.value); } = X
                );
    }

    void startUp( std::string && backtrace )
    {
        dl->dump( backtrace );
        closed = make_shared<sClosed>();
    }
    void closeEntry(){ dl->dump( "Closed(Entry)"); }
    void closeExit(){ dl->dump("Closed(Exit)"); }

    shared_ptr<cHelper> dl;
    std::string _name;
    shared_ptr<sClosed> closed;
};

int main()
{
    sml::sm<cDoorLogics> sm_impl{ make_shared<cHelper>(), "MyLogics" };
}
