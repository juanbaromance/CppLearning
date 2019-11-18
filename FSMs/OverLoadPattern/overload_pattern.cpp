#include <iostream>
#include "match.hpp"
#include <string>
#include <cmath>
#include <typeinfo>
#include <boost/format.hpp>

template <typename T>
struct crtp {
    T& underly(){ return static_cast<T&>(*this); }
    T const& underly() const { return static_cast<T const&>(*this); }
};
using namespace std;

template <typename T>
struct fsm_state : public crtp<fsm_state<T>>
{
    fsm_state() : trace_level(0) {}
    virtual bool onEntry(){ return true; }
    virtual void onExit( const string & backtrace = "" )
    {
        ( cout << boost::format("state( %20s ) : proc : %10s\n") % dump() % backtrace ).flush();
    }
    virtual const string dump(){ return this->underly().dump(); }
    int trace_level;
};

struct sClosed : fsm_state<sClosed> {
public:

    sClosed( )
    {
        // Major crosses filtering
        if( onEntry() == true )
            ( cout << boost::format("state( %20s )\n") % dump() ).flush();
    }
    void onExit( const string & backtrace = "" )
    {
        fsm_state::onExit( __FUNCTION__ + backtrace );
    }
    const string dump(){ return "Closed"; }
};

struct sOpened : fsm_state<sOpened> {

    sOpened( fsm_state<sClosed> & s )
    {
        // Prototypes general entry-state guard
        if( onEntry() )
        {
            s.onExit();
            ( cout << boost::format("state( %20s )\n") % dump() ).flush();
        }
    }
    void onExit( const string & backtrace = "" )
    {
        fsm_state::onExit( __FUNCTION__ + backtrace );
    }
    const string dump(){ return "Opened"; }
};

struct sLocked : fsm_state<sLocked> {
    sLocked( const string & keyword, fsm_state<sClosed> & s ) : keyword_ ( keyword )
    {
        // Prototypes entry-state guard
        if( onEntry() )
        {
            ( cout << boost::format("Locking Action: ")).flush();
            s.onExit();
            ( cout << boost::format("state( %20s )\n") % dump() ).flush();

        }
        else
            throw "forbidden entry invalid keyword";
    }
    bool onEntry()
    {
        // Entry guard modelling
        return ( keyword_ .length() < 10 ) ? false : true;
    }
    void onExit ( const string & backtrace = "" )
    {
        fsm_state::onExit( __FUNCTION__ + backtrace );
    }
    const string dump(){ return ( boost::format("Locked( %x )") % std::hash<std::string>{}(keyword_)).str(); }
    const string keyword(){ return keyword_ ; }
    string keyword_ ;
};


struct door_logics
{
    using State = variant <sOpened,sClosed,sLocked>;

    door_logics & eLock( const string & keyword )
    {
        using namespace variant_talk;
        state_ = match( state_ ,
        [=]( const sOpened & ) -> State
        { return dump( "CLOSE door before Locking").state(); },
        []( const sLocked & s ) -> State
        { return s; },
        [=]( sClosed & s ) -> State
        {
            // Todo below is the typical transition guard
            if( ! keyword.empty() )
            {
                try {
                    return sLocked( keyword, s );
                } catch ( char const* error )
                {
                    dump( ( boost::format("Locking Action: %s") % error ).str() );
                }
            }
            return s;
        }
        );
        return *this;
    }

    door_logics & eOpen( )
    {
        using namespace variant_talk;
        state_ = match( state_ ,
           []( sOpened & s ) -> State { return s; },
           []( sClosed & s ) -> State { return sOpened( s ); },
           [=]( sLocked &  ) -> State
        {
            return dump( "UnLock door before OPENING").state();
        }
        );
        return *this;
    }

    door_logics & eClose()
    {
        using namespace variant_talk;
        state_ = match( state_ ,
           []( sClosed & s ) -> State { return s; },
           []( sLocked & s ) -> State { return s; },
           []( sOpened & s ) -> State
        {
            s.onExit(); return sClosed();
        }
        );
        return *this;
    }

    door_logics & eUnLock( const string & keyword )
    {
        using namespace variant_talk;
        state_ = match( state_ ,
           []( sClosed & s ) -> State { return s; },
           []( sOpened & s ) -> State { return s; },
           [=]( sLocked & s ) -> State
        {
            if( keyword != s.keyword() )
            {
                std::size_t h1 = std::hash<std::string>{}(keyword);
                return dump( ( boost::format("UNLOCKING action rejected: Key %x doesn't match") % h1 ).str() ).state();
            }
            s.onExit();
            return sClosed();
        }
        );
        return *this;
    }

    State state_ { sClosed() };

    template<typename R = sOpened, typename S = sClosed, typename T = sLocked>
    door_logics & dump( string backtrace = "" )
    {
        using namespace variant_talk;
        string s = match( state_ ,
              []( R &s ) -> string { return s.dump(); },
              []( S &s ) -> string { return s.dump(); },
              []( T &s ) -> string { return s.dump(); }
        );
        ( cout << backtrace << " : " << s << "\n").flush();
        return *this;
    }

    State state(){ return state_ ; }
};

#include <bits/stdc++.h>
#include <functional>

constexpr static char magics[] = "12345678910";
static void fsm_testing( void )
{


    door_logics dl;
    auto tryClose  = [&dl](){ dl.eClose(); };
    auto tryOpen   = [&dl](){ dl.eOpen(); };
    auto tryUnLock = [&dl]( const string & key = magics ){ dl.eUnLock(key); };
    auto tryLock   = [&dl]( const string & key = magics ){ dl.eLock(key); };

    tryClose();

    // Open state testing
    dl.dump("\n## Opening Test");
    tryOpen();
    tryLock("1233");
    tryLock();
    tryClose();
    tryLock("22123");
    tryLock();

    // Locking state testing
    dl.dump("\n## Locking Test");
    tryOpen();
    tryClose();
    tryUnLock("123");
    tryUnLock();
    tryOpen();

}


static void match_testing();

int main()
{
    if ( /* DISABLES CODE */ (0) ) { match_testing(); }
    fsm_testing();
    return 0;
}


static void match_testing()
{
    using namespace std;
    variant<int, float, string> intFloatString { float(4.32) };
    float f_offset(-40.7);
    int i_offset(-10);

    intFloatString = int( 1 );

    /* the magics of pattern matching with sintatic sugar ala haskell */
    using namespace variant_talk;
    match( intFloatString,
          [=]( int& i ) mutable { i_offset = static_cast<int>(round( i_offset*0.6 )); i= i*2 + i_offset; },
          [=]( float& f ) { f = f*2.0f + f_offset; },
          [](string& ){}
    );

    /* Testing illegal state */
    try {
        intFloatString =
                string("This is the new three type stuff of c++17 ") +
                string("var holds (float): ") +
                to_string( get<float>(intFloatString));
    } catch ( const std::bad_variant_access& )
    {
        cout << "Error :: Projection <float> ( "
             << std::boolalpha
             << std::holds_alternative<float>(intFloatString)
             << " ) instance view "
             << "focused on offset("
             << intFloatString.index()
             << ")\n\n";
    }

    intFloatString =
            string("This is the new three type stuff of c++17 ") +
            string("var holds (int): ") +
            to_string( get<int>(intFloatString));

    match( intFloatString,
        [](const int& i) { cout << "int: " << i << '\n'; },
        [](const float& f) { cout << "float: " << f << '\n'; },
        [](const string& s) { cout << "string values: " << s << '\n';} );
}
