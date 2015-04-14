 #ifndef JUST_PTR_H
#define JUST_PTR_H

#include <atomic>

namespace sfl
{
    // because friend does not work. don't use this in client code
    class Unsafe
    {
    };


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

		template<typename Y>
		just_ptr &operator=(const just_ptr<Y> &rhs) 
		{
			if (_ptr == rhs._ptr) {
				return;
			}
			if (--(*_refCount) == 0) {
				delete _refCount;
				delete _ptr;
			}
			_refCount = rhs._refCount;
			_ptr = rhs._ptr;
			++(*_refCount);
		}

		~just_ptr()
		{
			if (--(*_refCount) == 0) {
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

	private:
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
}

#endif