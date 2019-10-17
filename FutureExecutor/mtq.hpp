#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>
#include <iostream>
#include <string>

template <typename T>
struct crtp {
    T& underly(){ return static_cast<T&>(*this); }
    T const& underly() const { return static_cast<T const&>(*this); }
};

template <typename T, typename Impl>
struct iMTQ : crtp<Impl> {
    void push( T t ){ this->underly().push( t ); }
    std::optional<T> pop( size_t delay = 0 ){ return this->underly().pop(delay); }
    void destroy(){ this->underly().destroy(); }
};

#include <chrono>
using MilliSeconds = std::chrono::milliseconds;
using Mutex = std::mutex;
using ConditionVariable = std::condition_variable;
using UniqueLock = std::unique_lock<std::mutex>;

template <typename T>
class MTQ : public iMTQ<T,MTQ<T>> {
public:
    MTQ( std::size_t max_size ) : max_size_(max_size) {}

    void push( T t )
    {
        UniqueLock lock{mut_};
        for (;;)
        {
            if (q_.size() < max_size_)
            {
                q_.push(std::move(t));
                cvar_.notify_all();
                return;
            } else
            {
                cvar_.wait(lock);
            }
        }
    }

    std::optional<T> pop( size_t delay = 0 )
    {
        UniqueLock lock{ mut_ };
        for (;;)
        {
            if (!q_.empty())
            {
                T t = q_.front();
                q_.pop();
                cvar_.notify_all();
                return std::move(t);
            }
            else
            {
                if (quit) return std::nullopt;
                if( delay == 0 )
                    cvar_.wait(lock);
                else
                {
                    cvar_.wait_for( lock, MilliSeconds(delay));
                    if( q_.empty() )
                        return std::optional<T>() ;
                }
            }
        }
    }

    bool done() const
    {
        UniqueLock lock{mut_};
        return quit;
    }

    void destroy()
    {
        UniqueLock lock{mut_};
        quit = true;
        cvar_.notify_all();
    }

private:
    std::size_t max_size_ = 0;
    bool quit = false;
    std::queue<T> q_;
    mutable Mutex mut_;
    mutable ConditionVariable cvar_;
};


namespace MTQTesting
{

template <class... T>
void auditor(std::string backtrace, T... arg)
{
    (std::cout << backtrace << std::endl).flush();
}

template <typename T = int>
struct Testing
{

    using QImpl = MTQ<T>;
    using QIface = std::unique_ptr<iMTQ<T, QImpl>>;
    void testQImpl( QIface &&q )
    {
        for ( auto k : { 0, 1, 2 } )
            q->push(k);
        std::optional<T> k;
        while ((k = q->pop(10)).has_value())
            auditor( __FUNCTION__ + std::string(":") + std::to_string(k.value()));
    }

    void ProbeMe(){ testQImpl( QIface( new QImpl(5)) ); }
};

} // namespace MTQTesting
