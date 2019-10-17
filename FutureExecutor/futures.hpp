#pragma once
#include "mtq.hpp"
#include <functional>
#include <memory>
#include <vector>
#include <thread>

class thread_pool {
public:
    thread_pool() : q_( std::numeric_limits<int>::max() - 1 ) { add_thread(); }
    ~thread_pool()
    {
	q_.set_done();
	for (auto& thread : threads_)
	    if (thread.joinable())
		thread.join();
    }

    void add( std::function<void()> f) { q_.push(std::move(f)); }
    void add_thread()
    {
	UniqueLock lock{mut_};
	threads_.emplace_back([this]() mutable {
	    while (true)
	    {
		auto f = q_.pop();
		if (f)
		{
		    (*f)();
		} else
		{
		    if (q_.done()) return;
		}
	    }
	});
    }

private:
    MTQ<std::function<void()>> q_;
    Mutex mut_;
    std::vector<std::thread> threads_;
};

template <typename T>
struct shared {
    T value;
    std::exception_ptr eptr = nullptr;
    Mutex mutex;
    ConditionVariable cvar;
    bool done = false;
    std::function<void()> then;
    std::shared_ptr<thread_pool> pool;
};

template <typename T>
class future {
public:
    void wait() {
	UniqueLock lock{shared_->mutex};
	while (!shared_->done) {
	    shared_->cvar.wait(lock);
	}
    }
    template <typename F>
    auto then(F f) -> future<decltype(f(*this))>;

    T& get() {
	wait();
	if (shared_->eptr) {
	    std::rethrow_exception(shared_->eptr);
	}
	return shared_->value;
    }

    explicit future(const std::shared_ptr<shared<T>>& shared) : shared_(shared) {}

private:
    std::shared_ptr<shared<T>> shared_;
};

template <typename T>
void run_then(UniqueLock lock, std::shared_ptr<shared<T>>& s) {
    std::function<void()> f;
    if (s->done) {
	std::swap(f, s->then);
    }
    lock.unlock();
    if (f) f();
}

template <typename T>
class promise {
public:
    promise() : shared_(std::make_shared<shared<T>>()) {}
    template <typename V>
    void set_value(V&& v) {
	UniqueLock lock{shared_->mutex};
	shared_->value = std::forward<V>(v);
	shared_->done = true;
	run_then(std::move(lock), shared_);
	shared_->cvar.notify_one();
	shared_ = nullptr;
    }
    void set_exception(std::exception_ptr eptr) {
	UniqueLock lock{shared_->mutex};
	shared_->eptr = eptr;
	shared_->done = true;
	run_then(std::move(lock), shared_);
	shared_->cvar.notify_one();
	shared_ = nullptr;
    }

    future<T> get_future() { return future<T>{shared_}; }

    explicit promise(const std::shared_ptr<shared<T>>& shared)
	: shared_(shared) {}

private:
    std::shared_ptr<shared<T>> shared_;
};

template <typename T>
template <typename F>
auto future<T>::then(F f) -> future<decltype(f(*this))> {
    UniqueLock lock{shared_->mutex};
    using type = decltype(f(*this));
    auto then_shared = std::make_shared<shared<type>>();
    then_shared->pool = shared_->pool;
    shared_->then = [shared = shared_, then_shared, f = std::move(f),
	    pool = shared_->pool]() mutable {
	pool->add([shared, then_shared, f = std::move(f),
		  p = promise<type>(then_shared)]() mutable {
	    future<T> fut(shared);
	    try {
		p.set_value(f(fut));
	    } catch (...) {
		p.set_exception(std::current_exception());
	    }
	});
    };
    run_then(std::move(lock), shared_);
    return future<type>(then_shared);
}

template <typename F, typename... Args>
auto async(std::shared_ptr<thread_pool> pool, F f, Args... args)
-> future<decltype(f(args...))> {
    using T = decltype(f(args...));
    auto state = std::make_shared<shared<T>>();
    state->pool = pool;
    promise<T> p(state);
    auto fut = p.get_future();
    auto future_func = [p = std::move(p), f = std::move(f), args...]() mutable {
	try {
	    p.set_value(f(args...));
	} catch (...) {
	    p.set_exception(std::current_exception());
	}
    };
    pool->add(std::move(future_func));
    return fut;
}
