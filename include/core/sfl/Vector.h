#ifndef SFL_VECTOR_H
#define SFL_VECTOR_H

#include <vector>
#include "sfl/Maybe.h"

namespace sfl
{
	namespace Vector
	{
		template<typename A>
		std::vector<A> singleton(const A &a)
		{
			return std::vector<A>{a};
		}
	}

	template<typename A>
	A at(const std::vector<A> &v, int index)
	{
		return v.at(index);
	}

	template<typename F,typename V,typename A = typename V::value_type>
	Maybe<A> find(F &&f, const V &v)
	{
		auto it = find_if(v.begin(), v.end(), f);
		if (it == v.end()) {
			return Nothing();
		}
		return *it;
	}

	template<typename A>
	Maybe<A> maybeAt(const std::vector<A> &v, int index)
	{
		if (index < v.size()) {
			return v.at(index);
		}
		return Nothing();
	}

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