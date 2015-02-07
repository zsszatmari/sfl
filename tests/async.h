#ifndef ASYNC_H
#define ASYNC_H

#include <stdplus.h>
#include CHRONO_H
#include MUTEX_H
#include LOCK_GUARD_H
#include CONDITION_VARIABLE_H
#include THREAD_H

using CHRONO_NS::milliseconds;
using THREAD_NS::mutex;
using THREAD_NS::lock_guard;
using THREAD_NS::condition_variable;
using THREAD_NS::unique_lock;
using std::function;

static void async(const function<void(const function<void()> &)> &f)
{
	mutex m;
	bool finished = false;
	condition_variable cond;

	auto done = [&]{
		lock_guard<mutex> l(m);
		finished = true;
		cond.notify_all();
	};

	THREAD_NS::thread thr([=]{
		f(done);
	});
	thr.detach();

	unique_lock<mutex> l(m);
	EXPECT_TRUE(cond.wait_for(l, milliseconds(500), [&]{return finished;}));
}

#endif