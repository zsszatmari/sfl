#ifndef SFL_IMMUTABLEVECTOR_H
#define SFL_IMMUTABLEVECTOR_H

#include <atomic>
#include <cassert>
#include "sfl/Prelude.h"

namespace sfl
{
	/**
	  * A reference counted vector with immutable values. Slicing it does not recquire a copy,
	  * which may be beneficial to performance.
	  */

	template<typename T>
	class ImmutableVector final
	{	
	public:
		typedef T value_type;

		class const_iterator final : public std::iterator<std::random_access_iterator_tag,T>
		{
		public:
			const_iterator(const ImmutableVector<T> &ref, size_t offset) :
				_ref(ref),
				_offset(offset)
			{
			}

			const T& operator*() const
			{
				return _ref._values[_offset];
			}

		    const T* operator->() const
		    {
		    	return _ref._values + _offset;
		    }

		    size_t operator-(const const_iterator &rhs) const
		    {
		    	return _offset - rhs._offset; 
		    }

		    bool operator==(const const_iterator &rhs) const
		    {
		    	return _offset == rhs._offset;
		    }

		    bool operator!=(const const_iterator &rhs) const
		    {
		    	return _offset != rhs._offset;
		    }

		    const_iterator & operator++()
		    {
		    	++_offset;
		    	return *this;
		    }

		    const_iterator & operator+=(int diff)
		    {                           
		    	_offset += diff;
			    return *this;
			}
			 
			friend const_iterator operator+(const_iterator lhs, int diff) 
			{
			    return lhs += diff;
		    }

		    bool operator<(const const_iterator &rhs)
		    {
		    	return _offset < rhs._offset;
		    }

		private:
			ImmutableVector<T> _ref;
			size_t _offset;

			friend class ImmutableVector<T>;
		};

		const_iterator begin() const
		{
			return const_iterator(*this, 0);
		}

		const_iterator end() const
		{
			return const_iterator(*this, _size);
		}

		size_t size() const
		{
			return _size;
		}

		const T & at(size_t pos) const
		{
			assert(pos < _size);
			return *(_values + pos);
		}

		ImmutableVector() :
			_refCount(new std::atomic_int(1)),
			_size(0),
			_values(new T[_size]),
			_originalValues(_values)
		{
		}

		ImmutableVector(const ImmutableVector<T> &rhs) :
			_refCount(rhs._refCount),
			_size(rhs._size),
			_values(rhs._values),
			_originalValues(_values)
		{
			++(*_refCount);
		}

		ImmutableVector<T> &operator=(const ImmutableVector<T> &rhs)
		{
			if (_originalValues != rhs._originalValues) {
				if (--(*_refCount) == 0) {
					delete _refCount;
					delete[] _originalValues;
				}
				_refCount = rhs._refCount;
				_originalValues = rhs._originalValues;
				++(*_refCount);
			}

			_size = rhs._size;
			_values = rhs._values;

			return *this;
		}

		ImmutableVector(const const_iterator &begin, const const_iterator &end) :
			_refCount(begin._ref._refCount),
			_originalValues(_values),
			_values(begin._ref._values + begin._offset),
			_size(end._offset - begin._offset)
		{
			// must refer to same container
			assert(begin._ref._values == end._ref._values);
			++(*_refCount);
		}

		~ImmutableVector()
		{
			if (--(*_refCount) == 0) {
				delete _refCount;
				delete[] _originalValues;
			}
		}

	private:
		int _size;
		T *_values;
		T *_originalValues;
		std::atomic_int *_refCount;


		ImmutableVector(int size) :
			_refCount(new std::atomic_int()),
			_size(size)
		{
			*_refCount = 1;
			// _values left uninitialized
		}
 
		friend class ImmutableVectorUtil;
	};

	struct ImmutableVectorUtil
	{
		template<typename R,typename T = typename R::value_type>
		static ImmutableVector<T> toImmutableVector(const R &range)
		{
			auto size = length(range);
			ImmutableVector<T> ret(size);
			ret._values = new T[size];
			ret._originalValues = ret._values;
			std::copy(range.begin(),range.end(), ret._values);
			return ret;
		}
	};

	template<typename R,typename T = typename R::value_type>
	ImmutableVector<T> toImmutableVector(const R &range)
	{
		return ImmutableVectorUtil::toImmutableVector(range);
	}
}

#endif
