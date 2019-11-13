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
    virtual void tick(){}
    virtual const string dump(){ return this->underly().dump(); }
    int trace_level;
};


struct sOpened {};

struct sClosed : fsm_state<sClosed> {
public:
    sClosed(){}
    void onExit( const string & backtrace = "" )
    {
        fsm_state::onExit( __FUNCTION__ );
    }
    void tick()
    {
        cout << __PRETTY_FUNCTION__ << "\n";
    }
    const string dump(){ return "Closed"; }
};

struct sLocked : fsm_state<sLocked> {
    sLocked( string keyword, fsm_state<sClosed> & f ) : keyword_ ( keyword )
    {
        // Prototypes entry-state guard
        if( onEntry() )
        {
            f.onExit();
            ( cout << boost::format("state( %20s )\n") % dump() ).flush();
        }
        else
            throw " forbidden entry : invalid keyword";
    }
    virtual bool onEntry(){ return ( keyword_ .length() < 10 ) ? false : true; }
    const string dump(){ return ( boost::format("Locked(%10s)") % keyword_ ).str(); }
    string keyword_ ;
};

using State = variant <sOpened,sClosed,sLocked>;


struct eClose {};
struct eOpen {};
struct eUnLock {};

struct lock_logics
{
    lock_logics & eLock( const string & keyword )
    {
        // Todo below is the typical transition guard
        if( ! keyword.empty() )
        {
            try {
                state_ = sLocked( keyword, get<sClosed>( state_ ) );
            } catch ( char const* error )
            {
                dump( ( boost::format(": %s:%s\n") % __FUNCTION__ % error ).str() );
            }
        }
        return *this;
    }

    State state_ { sClosed() };

    void Tick()
    {
        using namespace variant_talk;
        state_ = match( state_ ,
           [=]( sOpened & state ) -> State { },
           [=]( sClosed & state ) -> State { return sClosed(); },
           [=]( sLocked & state ) -> State { }
        );
    }

    template<typename R = sOpened, typename S = sClosed, typename T = sLocked>
    void dump( string backtrace = "" )
    {
        using namespace variant_talk;
        string s = match( state_ ,
              []( R &s ) -> string { return typeid(s).name(); },
              []( S &s ) -> string { return s.dump(); },
              []( T &s ) -> string { return s.dump(); }
        );
        ( cout << s << ( backtrace.empty() ? "" : ( ":" + backtrace )) ).flush();
    }

    State state(){ return state_ ; }
};

static void fsm_testing( void )
{
    lock_logics ll;
    ll.eLock("12345678910");
}



void match_testing()
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

int main()
{
    match_testing();
    fsm_testing();
    return 0;
}
