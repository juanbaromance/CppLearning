#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>

template <typename T>
struct crtp {
    T& underly(){ return static_cast<T&>(*this); }
    T const& underly() const { return static_cast<T const&>(*this); }
};

template <typename T, typename Impl>
struct iMTQ : crtp<Impl> {
    void push( T t ){ this->underly().push( t ); }
    std::optional<T> pop( size_t delay = 0 ){ return this->underly().pop(delay); }
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
		if (done_) return std::nullopt;
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
	return done_;
    }

    void set_done()
    {
	UniqueLock lock{mut_};
	done_ = true;
	cvar_.notify_all();
    }

private:
    std::size_t max_size_ = 0;
    bool done_ = false;
    std::queue<T> q_;
    mutable Mutex mut_;
    mutable ConditionVariable cvar_;
};
