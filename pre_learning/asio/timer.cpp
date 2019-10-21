//
// timer.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/format.hpp>
#include <boost/bind.hpp>
typedef boost::posix_time::ptime Time;

#include <chrono>
#include <iostream>
#include <ctime>
#include <cmath>
#include <iomanip>
#include <sys/time.h>
#include <thread>
#include <memory>
#include <mutex>
using UniqueLock = std::unique_lock<std::mutex>;
using Mutex = std::mutex;
static std::string snapshot()
{
    std::ostringstream oss;
    oss << " ( " << std::setw(5) << getpid( ) << " )( ";
    time_t t; time( & t );
    struct tm * tm = localtime( & t );
    struct timeval tv; gettimeofday( & tv, nullptr);
    oss << std::setfill ('0') << std::setw (2) << tm->tm_hour;
    oss << ":" << std::setfill ('0') << std::setw (2) << tm->tm_min;
    oss << ":" << std::setfill ('0') << std::setw (2) << tm->tm_sec;
    oss << ":" << std::setfill ('0') << std::setw (3) << static_cast<int>(round( tv.tv_usec / 1000 )) << " ) ";
    return oss.str( );
}


#include <tuple>
#include <map>
class iTimer {
public:

    using spec = std::tuple<bool,size_t>;

    template <typename ...T>
    void auditor( std::string backtrace, T... args )
    {
        UniqueLock lock{ mut_ };
        std::cout << boost::format("%s: %20s") % snapshot() % backtrace;
        ( std::cout << std::endl ).flush();
    }

    struct Subscriptor {
        virtual void timedOut( std::string ) = 0;
        virtual ~Subscriptor(){ }
    };

    using SubscriptorRef = std::weak_ptr<Subscriptor>;

    using Pointer = std::shared_ptr<iTimer>;
    virtual void Start( const spec && spec ) = 0;
    virtual void Subscribe( std::string id, SubscriptorRef w ) = 0;
    virtual ~iTimer(){ auditor( __PRETTY_FUNCTION__  ); }
    mutable Mutex mut_;

};

class cTimer : public iTimer {
public:
    enum class TraceLevel {
        Silent,
        Verbose,
    };
    TraceLevel trace_level = TraceLevel::Silent;

    cTimer( boost::asio::io_service &io, std::string name, size_t deadline ) :
        _synchronous(true), _name(name), t(io, std::chrono::seconds(deadline)), _deadline( deadline )
    {}

    ~cTimer() override { auditor( __PRETTY_FUNCTION__  ); }

private:
    void Start( const cTimer::spec && spec ) override
    {
        Start( std::get<0>(spec) );
        Start( std::get<1>(spec) );

        _start = boost::posix_time::microsec_clock::local_time();
        t.expires_from_now( std::chrono::seconds( _rt_deadline ));
        t.async_wait( boost::bind( & cTimer::callback, this ) );

        std::thread( [=](){
            std::ostringstream oss;
            oss << boost::format("%s : io_service") % __PRETTY_FUNCTION__ ;
            auditor( oss.str() );
            t.get_io_service().run();
        }
        ).join();

        if( trace_level > TraceLevel::Silent )
        {
            std::ostringstream oss;
            oss << boost::format("%s : done") % __PRETTY_FUNCTION__ ;
            auditor( oss.str() );
        }
    }

private:
    std::map<std::string, SubscriptorRef > w_observers;
    void Subscribe( std::string id, SubscriptorRef w ) override { w_observers[ id ] = w; }



    void Start( bool synchronous ){ _synchronous = synchronous; }
    void Start( size_t deadline )
    {
        _rt_deadline = ( deadline == 0 ) ? _deadline : deadline;
        if( trace_level > TraceLevel::Silent )
        {
            std::ostringstream oss;
            oss << boost::format("%s # %s %d(sec)")
                   % __PRETTY_FUNCTION__
                   % ( deadline == 0 ? "" : "re-programmed" )
                   % _rt_deadline;
            auditor( oss.str() );
        }
    }
    bool _synchronous;

    void callback()
    {
        Time end = boost::posix_time::microsec_clock::local_time();
        auditor( ( boost::format("%s # %d(msec)")
                   % __PRETTY_FUNCTION__  % (end - _start).total_milliseconds() ).str() );
        for( auto & [ k, w ] : w_observers )
        {
            if( w.lock() )
                w.lock()->timedOut( k );
            else
            {
                auditor( ( boost::format("%s # purgue %s obvserver") % __PRETTY_FUNCTION__ % k ).str() );
                w_observers.erase(k);
            }
        }
    }

private:
    std::string _name;
    boost::asio::steady_timer t;
    Time _start;
    size_t _deadline, _rt_deadline;
};


class cNotifier : public iTimer::Subscriptor, public std::enable_shared_from_this<cNotifier>
{
public:
    cNotifier( std::string _name, std::shared_ptr<iTimer> timer ) : w( timer ), name( _name ){}
    void Subscribe( ){ w.lock()->Subscribe( name, weak_from_this() ); }
    virtual ~cNotifier() override
    { if( w.lock() ) w.lock()->auditor( __PRETTY_FUNCTION__  ); }

private:
    void timedOut( std::string event ) override
    { if( w.lock() ) w.lock()->auditor( __PRETTY_FUNCTION__ + ( boost::format("(%s)") % event ).str() ); }
    std::weak_ptr<iTimer> w;
    std::string name;
};



int main()
{
    boost::asio::io_service io;

    iTimer::Pointer timer( std::make_shared<cTimer>(io, "MyTimer", 1) );

    auto notifier( std::make_shared<cNotifier>( __PRETTY_FUNCTION__ , timer ) );
    notifier->Subscribe();

    std::thread( [&](){ timer->Start({false, 5}); }).join();
    return 0;

}
