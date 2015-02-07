#ifndef STDPLUS_H
#define STDPLUS_H

// some std types are not available before c++11 so we should be able to use boost counterparts if necessary

namespace ClientDb
{
	#define THREAD_H <thread>
	#define MUTEX_H <mutex>
	#define MEMORY_H <memory>
	#define SHAREDFROMTHIS_H <memory>
	#define ATOMIC_H <atomic>
	#define CONDITION_VARIABLE_H <condition_variable>
	#define THREAD_NS std
	#define MEMORY_NS std
	#define ATOMIC_NS std
}

#endif