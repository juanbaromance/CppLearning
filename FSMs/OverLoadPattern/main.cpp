#include <iostream>
#include "match.hpp"
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

// Prints formatted error message.
#include "fmt/include/fmt/format.h"

template <typename T>
struct fsm_state : public crtp<fsm_state<T>>
{
    fsm_state() : trace_level(0) {}

    // Prototypes a scope-guard
    virtual bool onEntry(){ return true; }
    virtual void onExit( const string & backtrace = "" )
    { fmt::print("state( {0:10s} ) -> proc {1:10s}\n", dump(), backtrace ); }
    virtual const string dump(){ return this->underly().dump(); }
    int trace_level;
};

constexpr static char magics[] = "12345678910";
struct sClosed : fsm_state<sClosed> {
public:

    sClosed( )
    {
        // Todo : Model a global/specific Lock which disables constructor see EC++ item XY,
        // Very likely throw something is the best approach
        if( onEntry() == true )
            fmt::print("state( {0:10s} )\n", dump());
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
            fmt::print("state( {0:10s} )\n", dump());
        }
    }
    void onExit( const string & backtrace = "" ){ fsm_state::onExit( __FUNCTION__ + backtrace ); }
    const string dump(){ return "Opened"; }
};

struct sLocked : fsm_state<sLocked> {
    sLocked( const string & keyword, fsm_state<sClosed> & s ) : keyword_ ( keyword )
    {
        if( onEntry() )
        {
            fmt::print( "Locking Action: ");
            s.onExit();
            fmt::print( "Locking Action: {}", dump() );
        }
    }

    void onExit ( const string & backtrace = "" ){ fsm_state::onExit( __FUNCTION__ + backtrace ); }
    const string dump(){ return fmt::format("Locked( {0:x} )", std::hash<std::string>{}(keyword_) ); }
    const string keyword(){ return keyword_ ; }
    string keyword_ ;
};

using namespace variant_talk;

template <typename T>
struct iDoorLogics : public crtp<T>
{
public:
    void Open ( ){ this->underly().eOpen(); }
    void Close( ){ this->underly().eClose(); }
    void Lock( const string & keyword ){ this->underly().eLock( keyword ); }
    void UnLock( const string & keyword ){ this->underly().eUnLock( keyword ); }
};

struct cDoorLogics : iDoorLogics<cDoorLogics>
{
private:
    using State = variant <sOpened,sClosed,sLocked>;
    State state_ { sClosed() };
    State state(){ return state_ ; }

public:
    template<typename R = sOpened, typename S = sClosed, typename T = sLocked>
    cDoorLogics & dump( string backtrace = "" )
    {
        string report = match( state_ ,
              []( R &s ) -> string { return s.dump(); },
              []( S &s ) -> string { return s.dump(); },
              []( T &s ) -> string { return s.dump(); }
        );
        fmt::print("{0} : {1}\n" , backtrace, report );
        return *this;
    }

    void testing()
    {
        auto tryClose  = [=](){ eClose(); };
        auto tryOpen   = [=](){ eOpen(); };
        auto tryUnLock = [=]( const string & key = magics ){ eUnLock(key); };
        auto tryLock   = [=]( const string & key = magics ){ eLock(key); };

        tryClose();
        // Open state testing
        dump("\n## Opening Test");
        tryOpen();
        tryLock("1233");
        tryLock();
        tryClose();
        tryLock("22123");
        tryLock();

        // Locking state testing
        dump("\n## Locking Test");
        tryOpen();
        tryClose();
        tryUnLock("123");
        tryUnLock();
        tryOpen();
    }


    // Intrusive Approach A : Events manage the state implementation
private:

    void eOpen( )
    {
        state_ = match( state_ ,
           []( sClosed & s ) -> State { return sOpened( s ); },
           []( sOpened & s ) -> State { return s; },
           []( sLocked & s ) -> State { fmt::print("\"UNLOCK\" door before OPENING"); return s; }
        );
    }

    void eClose()
    {
        state_ = match( state_ ,
        // Todo : extra sintactic sugar is required to assembly below ala OR
        // ( sClosed | sLocked , s ) -> State { return s }
           []( auto s ) -> State { return s; },
           []( sOpened & s ) -> State { s.onExit(); return sClosed(); }
        );
    }

    void eLock( const string & keyword )
    {
        state_ = match( state_ ,
           []( sLocked & s  ) -> State { return s; },
           []( sOpened & s  ) -> State { fmt::print( "\"CLOSE\" door before \"LOCKING\"\n"); return s; },
           [=]( sClosed & s ) -> State {
            // Todo : Model below as a guard
            if( keyword.length() > 10 )
                return sLocked( keyword, s );
            return s;
        }
        );
    }

    void eUnLock( const string & keyword )
    {
        state_ = match( state_ ,
        []( auto s ) -> State { return s; },
        [=]( sLocked & s ) -> State
        {
            // Todo : Model below as a guard
            if( keyword == s.keyword() )
            {
                s.onExit();
                return sClosed();
            }
            fmt::print("\"UNLOCKING\" Action rejected: Key {0:x} doesn't match\n", std::hash<std::string>{}(keyword) );
            return s;
        }
        );
    }

    friend iDoorLogics<cDoorLogics>;

};

int main()
{
    if( /* DISABLES CODE */ (0) ){  cDoorLogics().testing();  }

    {
        auto idl = std::make_unique<cDoorLogics>();
        auto tryClose  = [&](){ idl->Close(); };
        auto tryOpen   = [&](){ idl->Open(); };
        auto tryUnLock = [&]( const string & key = magics ){ idl->UnLock(key); };
        auto tryLock   = [&]( const string & key = magics ){ idl->Lock(key); };

        idl->dump("\n## CRTP interface testing");

        tryClose();
        // Open state testing
        idl->dump("\n## Opening Test");
        tryOpen();
        tryLock("1233");
        tryLock();
        tryClose();
        tryLock("22123");
        tryLock();

        // Locking state testing
        idl->dump("\n## Locking Test");
        tryOpen();
        tryClose();
        tryUnLock("123");
        tryUnLock();
        tryOpen();
    }
    return 0;
}

