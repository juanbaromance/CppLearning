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
        if( onEntry() == true )
            ( cout << boost::format("state( %20s )\n") % dump() ).flush();
    }
    void onExit( const string & backtrace = "" ){ fsm_state::onExit( __FUNCTION__ + backtrace );  }
    const string dump(){ return "Closed"; }
};
struct sOpened : fsm_state<sOpened> {

    sOpened( fsm_state<sClosed> & s )
    {
        if( onEntry() )
        {
            s.onExit();
            ( cout << boost::format("state( %20s )\n") % dump() ).flush();
        }
    }
    void onExit( const string & backtrace = "" ){ fsm_state::onExit( __FUNCTION__ + backtrace ); }
    sOpened & auditor( const string & backtrace )
    {
        cout << boost::format("%s\n") % backtrace;
        return *this;
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

    bool onEntry() { return ( keyword_ .length() < 10 ) ? false : true; }
    void onExit ( const string & backtrace = "" ){ fsm_state::onExit( __FUNCTION__ + backtrace ); }
    sLocked & auditor( const string & backtrace )
    {
        cout << boost::format("%s\n") % backtrace;
        return *this;
    }

    const string dump(){ return ( boost::format("Locked( %x )") % std::hash<std::string>{}(keyword_)).str(); }
    const string keyword(){ return keyword_ ; }
    string keyword_ ;
};

using namespace variant_talk;
struct door_logics
{
private:
    using State = variant <sOpened,sClosed,sLocked>;
    State state_ { sClosed() };
    State state(){ return state_ ; }

public:
    template<typename R = sOpened, typename S = sClosed, typename T = sLocked>
    door_logics & dump( string backtrace = "" )
    {
        string report = match( state_ ,
              []( R &s ) -> string { return s.dump(); },
              []( S &s ) -> string { return s.dump(); },
              []( T &s ) -> string { return s.dump(); }
        );
        ( cout << backtrace << " : " << report << "\n").flush();
        return *this;
    }

    // Intrusive Approach A : Events manage the state implementation
public:
    using newState = std::optional<State>;

    door_logics & eOpen( )
    {
        state_ = match( state_ ,
           []( sClosed & s ) -> State { return sOpened( s ); },
           []( sOpened & s ) -> State { return s; },
           []( sLocked & s ) -> State { return s.auditor("\"UNLOCK\" door before OPENING"); }
        );
        return *this;
    }
    door_logics & eClose()
    {
        state_ = match( state_ ,
           []( sClosed & s ) -> State { return s; },
           []( sLocked & s ) -> State { return s; },
           []( sOpened & s ) -> State { s.onExit(); return sClosed(); }
        );
        return *this;
    }
    door_logics & eLock( const string & keyword )
    {
        using namespace variant_talk;
        state_ = match( state_ ,
        []( sLocked & s ) -> State { return s; },
        []( sOpened & s ) -> State { return s.auditor( "\"CLOSE\" door before \"LOCKING\""); },
        [=]( sClosed & s ) -> State
        {
            // Todo below is the typical transition guard
            if( ! keyword.empty() )
            {
                try {
                    return sLocked( keyword, s );
                } catch ( char const* error )
                {
                    dump( ( boost::format("\"LOCKING\" Action: %s") % error ).str() );
                }
            }
            return s;
        }
        );
        return *this;
    }
    door_logics & eUnLock( const string & keyword )
    {
        using namespace variant_talk;
        state_ = match( state_ ,
        [](  sClosed & s ) -> State { return s; },
        [](  sOpened & s ) -> State { return s; },
        [=]( sLocked & s ) -> State
        {
            if( keyword != s.keyword() )
            {
                std::size_t h1 = std::hash<std::string>{}(keyword);
                return dump( ( boost::format("\"UNLOCKING\" Action rejected: Key %x doesn't match") % h1 ).str() ).state();
            }
            s.onExit();
            return sClosed();
        }
        );
        return *this;
    }

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


int main()
{
    fsm_testing();
    return 0;
}

