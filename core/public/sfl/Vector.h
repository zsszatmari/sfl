#ifndef SFL_VECTOR_H
#define SFL_VECTOR_H

#include <vector>
#include "sfl/Maybe.h"
#include "sfl/Prelude.h"

namespace sfl
{
	namespace Vector
	{
		/*
		 * O(1) Vector with exactly one element
		 */
		template<typename A>
		std::vector<A> singleton(const A &a)
		{
			return std::vector<A>{a};
		}

		/**
		  * O(1) Empty vector
		  */
		template<typename A>
		std::vector<A> empty()
		{
			return std::vector<A>();
		}

		/**
		  * O(1) Empty vector. The arguement left untouched, only it's type interfered for convenience.
		  */
		template<typename A>
		std::vector<A> empty(const A &)
		{
			return std::vector<A>();
		}
	}

	/*
	 * Returns the element at index, or undefined behaviour if not present.
	 */
	template<typename A>
	A at(const std::vector<A> &v, int index)
	{
		return v.at(index);
	}

	/*
	 * Yield Just the first element matching the predicate or Nothing if no such element exists. 
	 */
	template<typename F,typename V,typename A = typename V::value_type>
	Maybe<A> find(F &&predicate, const V &v)
	{
		auto it = find_if(v.begin(), v.end(), predicate);
		if (it == v.end()) {
			return Nothing();
		}
		return *it;
	}

    /*
     * Return Just the element at index if present, or Nothing if not.
     */
	template<typename A>
	Maybe<A> maybeAt(const std::vector<A> &v, int index)
	{
		if (index < v.size()) {
			return v.at(index);
		}
		return Nothing();
	}

	/*
	 * Yield Just the index of the first occurence of the given element or Nothing if the 
	 * vector does not contain the element.
	 */
	template<typename A>
	Maybe<size_t> elemIndex(const A &elem, const std::vector<A> &v)
	{
		auto it = find(v.begin(),v.end(),elem);
		if (it == v.end()) {
            return Nothing();
		}
		return std::distance(v.begin(),it);
	}
}

#endif
