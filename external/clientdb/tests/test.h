#ifndef TEST_H
#define TEST_H

#include <functional>
#include <mutex>
#include <chrono>
#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using std::function;
using std::mutex;
using std::chrono::milliseconds;
using std::unique_lock;

inline void async(const function<void(const function<void()> &)> &f)
{
	mutex m;
	bool finished = false;
	std::condition_variable cond;

	auto done = [&]{
		std::lock_guard<mutex> l(m);
		finished = true;
		cond.notify_all();
	};

	std::thread thr([=]{
		f(done);
	});
	thr.detach();

	unique_lock<mutex> l(m);
	EXPECT_TRUE(cond.wait_for(l, milliseconds(500), [&]{return finished;}));
}

#endif