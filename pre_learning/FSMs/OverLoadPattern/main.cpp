#include <iostream>
#include "match.hpp"
#include <string>
#include <memory>
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

    // Prototypes a scope-guard
    virtual bool onEntry(){ return true; }
    virtual void onExit( const string & backtrace = "" )
    {  ( cout << boost::format("state( %20s ) : proc : %10s\n") % dump() % backtrace ).flush(); }
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
        if( onEntry() )
        {
            ( cout << boost::format("Locking Action: ")).flush();
            s.onExit();
            ( cout << boost::format("state( %20s )\n") % dump() ).flush();
        }
    }

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
        ( cout << backtrace << " : " << report << "\n").flush();
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
    using newState = std::optional<State>;

    void eOpen( )
    {
        state_ = match( state_ ,
           []( sClosed & s ) -> State { return sOpened( s ); },
           []( sOpened & s ) -> State { return s; },
           []( sLocked & s ) -> State { return s.auditor("\"UNLOCK\" door before OPENING"); }
        );
    }

    void eClose()
    {
        state_ = match( state_ ,
        // Todo : extra sintactic sugar is required to assembly below ala OR
           []( sClosed & s ) -> State { return s; },
           []( sLocked & s ) -> State { return s; },
           []( sOpened & s ) -> State { s.onExit(); return sClosed(); }
        );
    }

    void eLock( const string & keyword )
    {
        using namespace variant_talk;
        state_ = match( state_ ,
           []( sLocked & s  ) -> State { return s; },
           []( sOpened & s  ) -> State { return s.auditor( "\"CLOSE\" door before \"LOCKING\""); },
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
        using namespace variant_talk;
        state_ = match( state_ ,
        [](  sClosed & s ) -> State { return s; },
        [](  sOpened & s ) -> State { return s; },
        [=]( sLocked & s ) -> State
        {
            // Todo : Model below as a guard
            if( keyword == s.keyword() )
            {
                s.onExit();
                return sClosed();
            }
            std::size_t h1 = std::hash<std::string>{}(keyword);
            return dump( ( boost::format("\"UNLOCKING\" Action rejected: Key %x doesn't match") % h1 ).str() ).state();
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

