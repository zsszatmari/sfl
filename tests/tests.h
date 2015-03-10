#ifndef TESTS_H
#define TESTS_H

#include "stdplus.h"
#include CHRONO_H
#include LOCK_GUARD_H
#include RECURSIVE_MUTEX_H
#include THREAD_H
using CHRONO_NS::milliseconds;
using THREAD_NS::lock_guard;
using THREAD_NS::this_thread::sleep_for;
using THREAD_NS::recursive_mutex;
using std::function;

#endif
