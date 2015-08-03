 #ifndef JUST_PTR_H
#define JUST_PTR_H

#include <atomic>
#include "sfl/Maybe.h"

namespace sfl
{
    // because friend does not work. don't use this in client code
    class Unsafe
    {
    };

    template<class T>
    class mutable_just_ptr;


	/**
	  * A smart pointer, similar to shared_ptr, but it is always guaranteed that the value behind it is valid.
	  */
	template<typename T>
	class just_ptr final
	{
	public:
		just_ptr() = delete;

		template<typename Y>
		just_ptr(const just_ptr<Y> &rhs) :
			_refCount(rhs._refCount),
			_ptr(rhs._ptr)
		{
			++(*_refCount);
		}

		just_ptr(const just_ptr &rhs) :
			_refCount(rhs._refCount),
			_ptr(rhs._ptr)
		{
			++(*_refCount);
		}

		template<typename Y>
		just_ptr &operator=(const just_ptr<Y> &rhs) 
		{
			copyassign(rhs);
			
			return *this;
		}

		just_ptr &operator=(const just_ptr &rhs) 
		{
			copyassign(rhs);
			
			return *this;
		}

		~just_ptr()
		{
			if (_refCount->fetch_sub(1) == 1) {
				delete _refCount;
				delete _ptr;
			}
		}

		const T &operator*() const
		{
			return *_ptr;
		}

	    const T *operator->() const
	    {
	    	return _ptr;
	    }

	    const T *get() const
	    {
	    	return _ptr;
	    }

		just_ptr(const Unsafe &, T *ptr) :
			_ptr(ptr),
			_refCount(new std::atomic_int())
		{
			*_refCount = 1;
		}

		just_ptr(const Unsafe &, const mutable_just_ptr<T> &rhs) :
			_refCount(rhs._refCount),
			_ptr(rhs._ptr)
		{
			++(*_refCount);
		}

	private:
		template<typename Y>
		void copyassign(const just_ptr<Y> &rhs)
		{
			if (_ptr == rhs._ptr) {
				return;
			}
			if (_refCount->fetch_sub(1) == 1) {
				delete _refCount;
				delete _ptr;
			}
			_refCount = rhs._refCount;
			_ptr = rhs._ptr;
			_refCount->fetch_add(1);
		}

		std::atomic_int *_refCount;
		T *_ptr;

		template<typename Y>
		friend class just_ptr;
	};

	template<typename T,typename... Args>
	just_ptr<T> make_just(Args... args)
	{
		return just_ptr<T>(Unsafe(), new T(args...));
	}

	template<class T>
	class mutable_weak_just_ptr;

	/**
	  * A smart pointer, similar to shared_ptr, but it is always guaranteed that the value behind it is valid. On the 
	  * other hand, the value stored is mutable, and you must ensure that it is not accessed from multiple threads.
	  * Typical use case is to pass around large data.
	  */
	template<typename T>
	class mutable_just_ptr final
	{
	public:
		mutable_just_ptr() = delete;

		template<typename Y>
		mutable_just_ptr(const mutable_just_ptr<Y> &rhs) :
			_refCount(rhs._refCount),
			_weakRefCount(rhs._weakRefCount),
			_ptr(rhs._ptr)
		{
			_refCount->fetch_add(1);
		}

		mutable_just_ptr(const mutable_just_ptr &rhs) :
			_refCount(rhs._refCount),
			_weakRefCount(rhs._weakRefCount),
			_ptr(rhs._ptr)
		{
			_refCount->fetch_add(1);
		}

		template<typename Y>
		mutable_just_ptr &operator=(const mutable_just_ptr<Y> &rhs) 
		{
            copyassign(rhs);
			return *this;
		}

		mutable_just_ptr &operator=(const mutable_just_ptr &rhs)
		{
			copyassign(rhs);
			return *this;
		} 

		bool operator==(const mutable_just_ptr &rhs) const
		{
			return _ptr == rhs._ptr;
		}

		~mutable_just_ptr()
		{
			dispose();
		}

		T &operator*() const
		{
			return *_ptr;
		}

	    T *operator->() const
	    {
	    	return _ptr;
	    }

	    T *get() const
	    {
	    	return _ptr;
	    }

		mutable_just_ptr(const Unsafe &, T *ptr) :
			_ptr(ptr),
			_refCount(new std::atomic_int()),
            _weakRefCount(new std::atomic_int())
		{
			*_refCount = 1;
			*_weakRefCount = 0;
		}

		mutable_just_ptr(const Unsafe &, const mutable_weak_just_ptr<T> &rhs) :
			_refCount(rhs._refCount),
			_weakRefCount(rhs._weakRefCount),
			_ptr(rhs._ptr)
		{
			_refCount->fetch_add(1);
		}

	private:
		template<typename Y>
		void copyassign(const mutable_just_ptr<Y> &rhs) 
		{
			if (_ptr == rhs._ptr) {
				return;
			}
			assert(_refCount != rhs._refCount);
			dispose();

			_refCount = rhs._refCount;
			_weakRefCount = rhs._weakRefCount;
			_ptr = rhs._ptr;
			_refCount->fetch_add(1);
		}

		void dispose()
		{
			if (_refCount->fetch_sub(1) == 1) {
				delete _ptr;
				if (_weakRefCount == 0) {
					delete _refCount;
					delete _weakRefCount;
				}
			}
		}

		std::atomic_int *_refCount;
		std::atomic_int *_weakRefCount;
		T *_ptr;

		template<typename Y>
		friend class mutable_just_ptr;
		friend class just_ptr<T>;
		friend class mutable_weak_just_ptr<T>;
	};

	template<typename T,typename... Args>
	mutable_just_ptr<T> make_mutable_just(Args... args)
	{
		return mutable_just_ptr<T>(Unsafe(), new T(args...));
	}

	template<typename T>
	class mutable_weak_just_ptr final
	{
	public:
		mutable_weak_just_ptr() :
			_refCount(nullptr),
			_weakRefCount(nullptr),
			_ptr(nullptr)			
		{
		}

		mutable_weak_just_ptr(const mutable_just_ptr<T> &rhs) :
			_refCount(rhs._refCount),
			_weakRefCount(rhs._weakRefCount),
			_ptr(rhs._ptr)
		{
			_weakRefCount->fetch_add(1);
		}

		template<typename Y>
		mutable_weak_just_ptr(const mutable_weak_just_ptr<Y> &rhs) :
			_refCount(rhs._refCount),
			_weakRefCount(rhs._weakRefCount),
			_ptr(rhs._ptr)
		{
			_weakRefCount->fetch_add(1);
		}

		mutable_weak_just_ptr(const mutable_weak_just_ptr &rhs) :
			_refCount(rhs._refCount),
			_weakRefCount(rhs._weakRefCount),
			_ptr(rhs._ptr)
		{
			_weakRefCount->fetch_add(1);
		}

		~mutable_weak_just_ptr()
		{
			dispose();
		}

		template<typename Y>
		mutable_weak_just_ptr &operator=(const mutable_weak_just_ptr<Y> &rhs) 
		{
			copyassign(rhs);
			return *this;
		}

		mutable_weak_just_ptr &operator=(const mutable_weak_just_ptr &rhs)
		{
			copyassign(rhs);
			return *this;
		} 

		Maybe<mutable_just_ptr<T>> lock() const
		{
			if (!_refCount) {
				return Nothing();
			}
			if (*_refCount > 0) {
				return mutable_just_ptr<T>(Unsafe(), *this);
			} else {
				return Nothing();
			} 
		}

	private:
		template<typename Y>
		void copyassign(const mutable_weak_just_ptr<Y> &rhs) 
		{
			if (_ptr == rhs._ptr) {
				return;
			}
			if (_refCount) {
				assert(_refCount != rhs._refCount);
				dispose();
			}

			_refCount = rhs._refCount;
			_weakRefCount = rhs._weakRefCount;
			_ptr = rhs._ptr;
			_weakRefCount->fetch_add(1);
		}

		void dispose()
		{
			if (_weakRefCount && _weakRefCount->fetch_sub(1) == 1) {
				if (_refCount == 0) {
					delete _refCount;
					delete _weakRefCount;
				}
			}
		}

		friend class mutable_just_ptr<T>;

		std::atomic_int *_refCount;
		std::atomic_int *_weakRefCount;
		T *_ptr;
	};
}

#endif