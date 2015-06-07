#ifndef SFL_SPLIT_H
#define SFL_SPLIT_H

#include <vector>
#include <cassert>
#include "sfl/Prelude.h"
#include "sfl/ImmutableList.h"

namespace sfl
{
	/**
	  * chunk(n,range) splits a range into length/n pieces. The last piece will be shorter if n does 
	  * not evenly divide the length of the list.
	  */
	template<typename R>
	std::vector<R> chunk(size_t n, const R &r)
	{
		std::vector<R> ret;
		ret.reserve((r.size() + n -1) / n);
		int pos = 0;
		while ((r.size() - pos) >= n) {
			ret.push_back(R(r.begin() + pos, r.begin() + pos + n));
			pos += n;
		}
		if ((r.size() - pos) > 0) {
			ret.push_back(R(r.begin() + pos, r.end()));
		}
		return std::move(ret);
	}

	/**
	  * chunk(n,range) splits a range into length-n pieces. The last piece will be shorter if n does 
	  * not evenly divide the length of the list.
	  * Note that if R is an std::vector, this takes O(n^2) copies, while if an ImmutableVector
	  * is supplied, there is no copy at all, while calling toImmutableVector takes O(n)
	  *
	  * This is a recursive version. If stack overflow is a concern, avoid it.
	  */ 
	template<typename R>
	ImmutableList<R> chunkR(size_t n, const R &r)
	{
		return length(r) <= n 
					? List::singleton(r)
					: cons(take(n,r), chunkR(n, drop(n,r)));  
	}

	/**
	  * Split on the given sublist. Equivalent to split . dropDelims . onSublist. For example:
	  * splitOn(string(".."),string("a..b...c....d..")) == ["a","b",".c","","d",""]
	  */
	template<typename R>
	std::vector<R> splitOn(const R &on, const R &r)
	{
		if (on.empty()) {
			return std::vector<R>({r});
		}

		std::vector<R> ret;
		auto itLastEnd = r.begin();
		for (auto it = r.begin() ; (it + on.size()) <= r.end();) {
			bool last = (it + on.size()) == r.end();
			if (R(it, it+on.size()) == on) {
				ret.push_back(R(itLastEnd, it));
				if (last && on.empty()) {
					break;
				}
				it += on.size();
				itLastEnd = it;
				if (last) {
					break;
				}
			} else {
				if (last) {
					break;
				}
				++it;
			}
		}
		assert(itLastEnd <= r.end());
		ret.push_back(R(itLastEnd, r.end()));
		return ret;
	}
}

#endif
