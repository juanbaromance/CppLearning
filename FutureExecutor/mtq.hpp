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


namespace ContainerDisplay
{
  template<typename T, template<class,class...> class C, class... Args>
  std::ostream& operator <<(std::ostream& os, const C<T,Args...>& objs)
  {
    for ( auto const & obj : objs )
      os << obj << ' ';
    return os;
  }

};


#include <future>
#include <random>

namespace MTQTesting
{

template <typename T = int>
struct Testing
{

    using QImpl = MTQ<T>;
    using QIface  = std::unique_ptr<iMTQ<T, QImpl>>;
    using QSIface = std::shared_ptr<iMTQ<T, QImpl>>;
    mutable Mutex mtx;

    void testQImpl( QIface &&q )
    {
        for ( auto k : { 0, 1, 2 } )
            q->push(k);

        std::optional<T> k;
        std::vector<T> tmp;
        while ((k = q->pop(10)).has_value()) tmp.emplace_back(k.value());

        using namespace ContainerDisplay;
        std::cout << __FUNCTION__ << " : " << tmp << std::endl;
    }

    void Plain(){ testQImpl( QIface( new QImpl(5)) ); }

    void Async()
    {
        QSIface q( new QImpl(5));

        using QPage = std::vector<T>;
        std::future<QPage> pull,push;

        push = std::async( std::launch::async, [&]()
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(5,40);
            QPage page;

            for ( auto k : { 0, 1, 2, 3, 4 } )
            {
                std::this_thread::sleep_for(MilliSeconds( dis(gen) ) );
                page.emplace_back(k);
                q->push(k);
            }

            using namespace ContainerDisplay;
            UniqueLock lock{mtx};
            ( std::cout << __FUNCTION__ << " : " << page << " : page Updated" << std::endl ).flush();
            return page;
        });


        pull = std::async( std::launch::async, [&]()
        {
            push.wait_for(MilliSeconds(100));
            QPage floating_page;
            std::optional<T> k;
            while ( ( k = q->pop(10) ).has_value() )
                floating_page.emplace_back(k.value());
            return floating_page;
        });

        {
            std::future_status status = pull.wait_for(MilliSeconds(200));
            push.wait();

            using namespace ContainerDisplay;
            UniqueLock lock{mtx};
            ( std::cout << __FUNCTION__ << " # "
                        << " promised( "  << push.get() << ")"
                        << " vs "
                        << " effective( " << pull.get() << ")"
                        << std::endl ).flush();
        }


    }


};


} // namespace MTQTesting
