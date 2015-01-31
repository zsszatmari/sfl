#ifndef SFL_PRELUDE_H
#define SFL_PRELUDE_H

#include <vector>
#include <functional>
#include <type_traits>
#include "Maybe.h"

namespace sfl
{
    template<typename F,typename R,typename A = typename R::value_type,typename B = typename std::result_of<F(A &&)>::type>
    std::vector<B> map(F &&f, const R &range)
    {
        std::vector<B> ret;
        ret.reserve(std::distance(range.begin(),range.end()));
        transform(range.begin(), range.end(), back_inserter(ret), f);
        return std::move(ret);
    }

    template<typename A,typename R>
    Maybe<size_t> elemIndex(const A &elem, const R &range)
    {
    	auto it = find(range.begin(),range.end(),elem);
    	if (it == range.end()) {
    		return Nothing();
    	}
    	return std::distance(range.begin(),it);
    }
    
	template<typename A,typename B>
	A fst(const std::pair<A,B> &p)
	{
		return p.first;
	}

	template<typename A,typename B>
	B snd(const std::pair<A,B> &p)
	{
		return p.second;
	}

    template<typename R>
    R operator+(const R &lhs, const R &rhs)
    {
        R ret;
        ret.reserve(std::distance(lhs.begin(),lhs.end()) + std::distance(rhs.begin(),rhs.end()));
        std::copy(lhs.begin(),lhs.end(),back_inserter(ret));
        std::copy(rhs.begin(),rhs.end(),back_inserter(ret));
        return std::move(ret);
    }

    template<typename R,typename T = typename R::value_type>
    R snoc(const R &lhs, const T &rhs)
    {
        R ret;
        ret.reserve(std::distance(lhs.begin(),lhs.end()) + 1);
        std::copy(lhs.begin(),lhs.end(),back_inserter(ret));
        ret.push_back(rhs);
        return std::move(ret);
    }

    template<typename F,typename R,typename T = typename R::value_type>
    R filter(F &&f, const R &range)
    {
        R ret;
        std::copy_if(range.begin(),range.end(),back_inserter(ret),f);
        return std::move(ret);
    }

    template<typename F,typename R>
    std::pair<R,R> partition(F &&f, const R &range)
    {
        std::pair<R,R> ret;
        std::partition_copy(range.begin(),range.end(),back_inserter(ret.first),back_inserter(ret.second),f);
        return std::move(ret);
    }

    template<typename T, typename R>
    R intersperse(const T &innerElement, const R &range)
    {
        R ret;
        bool first = true;
        for (auto &element : range) {
            if (first) {
                first = false;
            } else {
                ret.push_back(innerElement);
            }
            ret.push_back(element);
        }
        return std::move(ret);
    }

    template<typename I, typename R = typename I::value_type>
    R concat(const I &range)
    {
        R ret;
        for (const R &element : range) {
            copy(element.begin(), element.end(), back_inserter(ret));
        }
        return std::move(ret);
    }

    template<typename T, typename R>
    T intercalate(const T &innerElement, const R &range)
    {
        return concat(intersperse(innerElement, range));
    }
}

#endif
