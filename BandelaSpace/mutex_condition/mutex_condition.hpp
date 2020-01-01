#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <thread>
#include <iostream>
#include <sstream>

void auditor(const char* format)
{
    std::cout << format;
}

template <typename...Args>
void auditor( std::string backtrace, Args...args)
{

    ( std::cout << backtrace << " : " );
    ( std::cout << std::endl ).flush();

}

template <typename T>
class mtq {
public:
    mtq(int max_size) : max_size_(max_size) {}

    void push(T t) {
	std::unique_lock<std::mutex> lock{mut_};
	for (;;) {
	    if (q_.size() < max_size_) {
		q_.push(std::move(t));
		cvar_.notify_all();
		return;
	    } else {
		cvar_.wait(lock);
	    }
	}
    }

    std::optional<T> pop() {
	std::unique_lock<std::mutex> lock{mut_};
	for (;;) {
	    if (!q_.empty()) {
		T t = q_.front();
		q_.pop();
		cvar_.notify_all();
		return t;
	    } else {
		if (done_) return std::nullopt;
		cvar_.wait(lock);
	    }
	}
    }

    bool done() const {
	std::unique_lock<std::mutex> lock{mut_};
	return done_;
    }

    void set_done() {
	std::unique_lock<std::mutex> lock{mut_};
	done_ = true;
	cvar_.notify_all();
    }

    size_t size() const { return q_.size(); }

private:
    int max_size_ = 0;
    bool done_ = false;
    std::queue<T> q_;
    mutable std::mutex mut_;
    mutable std::condition_variable cvar_;
};

class latch {
public:
    latch(int counter) : counter_(counter)
    {
	std::ostringstream oss;
	oss << __PRETTY_FUNCTION__ << ":" << std::to_string(counter);
	auditor( oss.str() );
    }

    void wait() {
	std::unique_lock<std::mutex> lock{mut_};
	for (;;) {
	    if (counter_ <= 0) {
		return;
	    } else {
		cvar_.wait(lock);
	    }
	}
    }

    void count_down(std::ptrdiff_t n = 1) {
	std::unique_lock<std::mutex> lock{mut_};
	if (counter_ > 0) {
	    counter_ -= n;
	}
	cvar_.notify_all();
    }

private:
    int counter_ = 0;
    mutable std::mutex mut_;
    mutable std::condition_variable cvar_;
};

class thread_group {
public:
    template <typename... Args>
    void push(Args &&... args) { threads_.emplace_back(std::forward<Args>(args)...); }
    void join_all()
    {
	for (auto &thread : threads_) {
	    if (thread.joinable()) {
		thread.join();
	    }
	}
    }

    ~thread_group() { join_all(); }

private:
    std::vector<std::thread> threads_;
};

std::vector<char> compress(const std::vector<char> &in) {
    std::this_thread::sleep_for(std::chrono::seconds(rand() % 5 + 1));
    return in;
}

std::vector<char> read(std::istream &is, size_t n) {
    std::vector<char> in(n);
    if (!is.read(in.data(), n)) {
	in.resize(is.gcount());
    }
    return in;
}

void write(std::ostream &os, const std::vector<char> &out) {
    os.write(out.data(), out.size());
}

using block_q = mtq<std::pair<int, std::vector<char>>>;

void reader(block_q &reader_q, mtq<int> &back_pressure, std::istream &is);

void compressor(block_q &reader_q, block_q &writer_q, latch &l) {

    for (;;)
    {
	auto block = reader_q.pop();
	if (!block) {
	    l.count_down();
	    return;
	}
	writer_q.push({block->first, compress(block->second)});
    }
}

void writer_q_closer(block_q &writer_q, latch &l) {
    l.wait();
    writer_q.set_done();
}

struct in_order_comparator {
    template <typename T>
    bool operator()(T &a, T &b) {
	return a.first > b.first;
    }
};

void in_order_writer(block_q &writer_q, mtq<int> &back_pressure,
		     std::ostream &os) {
    int counter = 0;
    std::priority_queue<std::pair<int, std::vector<char>>,
	    std::vector<std::pair<int, std::vector<char>>>,
	    in_order_comparator>
	    pq;
    for (;;) {
	while (!pq.empty() && pq.top().first == counter) {
	    write(os, pq.top().second);
	    back_pressure.push(1);
	    ++counter;
	    pq.pop();
	}
	auto block = writer_q.pop();
	if (!block) return;
	pq.push(std::move(*block));
    }
}
