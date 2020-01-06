#pragma once

#include <functional>
template <typename T>
using Func = std::function<T()>;

#include <chrono>
using Seconds  = std::chrono::seconds;
using Millisec = std::chrono::milliseconds;

struct nullpattern{};
static constexpr auto norm = []( auto arg ) -> char* { return const_cast<char*>(arg); };

#include <cstddef>
template <typename T>
constexpr auto enumCollapse(T val) { return static_cast<size_t>(val); }

#include <utility>
template <typename ... F>
struct overload_set : public F ... {
    overload_set( F&& ... f) : F(std::forward<F>(f)) ... {}
    using F::operator() ... ;
};

// Below function template roles as a factory of variadic lambdas
template <typename ... F>
auto overload( F && ... f ){ return overload_set<F...>( std::forward<F>(f)... ); }

#include <variant>
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename Variant, typename... Matchers>
auto match(Variant&& variant, Matchers&&... matchers)
{
    return std::visit(
                overloaded{std::forward<Matchers>(matchers)...},
                std::forward<Variant>(variant));
}

/* Dynamic reactor stallen from patterns.h gpio utility */
#include <map>
#include <thread>
enum
{
    Threaded,
    SingleStack
};

template <class T1, class T2, bool framing = SingleStack, typename...Args>
class  cReactor {

private:
    T2* context;
    typedef int (T2::*Method)( Args... args);

public:
    cReactor( T2* _context) : context( _context ) {}
    std::map<T1,Method> commands;
    int process( T1 probe_command, Args...args )
    {
        if( commands.find( probe_command ) == commands.end() )
            return -1;
        Method  m = commands.find( probe_command )->second;
        if( framing == Threaded )
            std::thread( m, context, args... ).detach();
        else
            return ( context->*m )( args... );
        return 0;
    }
};

#include <bitset>
template <size_t T, typename ... Events>
std::bitset<T> maskGenerator( Events...events )
{
    std::bitset<T> m(0);
    for( auto i : { events... } )
        m.set( i );
    return m;
}

template <typename ... Events>
constexpr size_t maskGenerator( Events...events )
{
    size_t m(0);
    for( auto i : { events... } )
        m |= 1 << i;
    return m;
}

#include <memory>
#include <iomanip>
enum Numerology {
    Silent,
    Verbose,
    Warning,
    Error
};

/*
 * JB#31122019
 * Second template parameter solves the diamond corner case of use, see Boccara on CRTP
 * This expands the concrete interface layer on the crtp layer,
 * so the end point of the hierarchy is NOT aliased and the diamond is broken
 * This extra stuff allows to build Specialized classes assembled with different
 * crtps, composition taste is powered.
*/
template <typename T, template<typename> class crtpType>
struct crtp {
    T& underly(){ return static_cast<T&>(*this); }
    T const& underly() const { return static_cast<T const&>(*this); }
private:
    crtp(){}
    friend crtpType<T>;
};

#include <iomanip>
#include <sys/time.h>
#include <cmath>
#include <iostream>
#include <sys/unistd.h>
template <typename T=void>
class Module : public crtp<T,Module> {

public:
    const std::string configuration(){ return config; }
    int auditor( const std::stringstream & ss, const Numerology & severity = Verbose ) const
    {
        using namespace std;
        ostringstream oss;
        oss << " ( " << setw(5) << getpid( ) << " )( ";
        time_t t; time( & t );
        struct tm * tm = localtime( & t );
        struct timeval tv; gettimeofday( & tv, 0);
        oss << setfill ('0') << setw (2) << tm->tm_hour;
        oss << ":" << setfill ('0') << setw (2) << tm->tm_min;
        oss << ":" << setfill ('0') << setw (2) << tm->tm_sec;
        oss << ":" << setfill ('0') << setw (3) << static_cast<int>(round( tv.tv_usec / 1000 )) << " ) ";
        ( cout << ( ( severity > Verbose ) ? " WARNING " : "" ) << oss.str() << " " << ss.str() << "\n" ).flush();
        return ( severity > Verbose ) ? -1 : 0;
    }

private:
    Module( const std::string & module_config ) : config( module_config ){}
    friend T;
    std::string config;
};

#define define_has_member(member_name)                                         \
    template <typename T>                                                      \
    class has_member_##member_name                                             \
    {                                                                          \
        typedef char yes_type;                                                 \
        typedef long no_type;                                                  \
        template <typename U> static yes_type test(decltype(&U::member_name)); \
        template <typename U> static no_type  test(...);                       \
    public:                                                                    \
        static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes_type);  \
    };


/// static constexpr bool value = false;
///
/// Shorthand for testing if "class_" has a member called "member_name"
///
/// @note "define_has_member(member_name)" must be used
///       before calling "has_member(class_, member_name)"
#define has_member(class_, member_name)  has_member_##member_name<class_>::value


// Boccara wrapper to collapse for_each even more see rationale
// https://www.fluentcpp.com/2018/03/30/is-stdfor_each-obsolete/
#include <algorithm>
namespace ranges
{

template<typename Range, typename Function>
Function for_each(Range& range, Function f){ return std::for_each(begin(range), end(range), f); }

}
