#ifndef SFL_SPLIT_H
#define SFL_SPLIT_H

#include "sfl/Prelude.h"
#include "sfl/ImmutableList.h"

namespace sfl
{
	/**
	  * chunk(n,range) splits a range into length-n pieces. The last piece will be shorter if n does 
	  * not evenly divide the length of the list.
	  * Note that if R is an std::vector, this takes O(n^2) copies, while if an ImmutableVector
	  * is supplied, there is no copy at all, while calling toImmutableVector takes O(n)
	  *
	  * This is a recursive operation, if stack overflow is a concern, avoid it.
	  */ 
	template<typename R>
	ImmutableList<R> chunkR(size_t n, const R &r)
	{
		return length(r) <= n 
					? List::singleton(r)
					: cons(take(n,r), chunkR(n, drop(n,r)));  
	}
}

#endif
