#ifndef SFL_RANGE_H
#define SFL_RANGE_H

#include "sfl/Maybe.h"

/** Note the terminology 'range' refers to a genericcontainer. Requirements vary,
  * but it must have a begin() and end() iterators, and many times a push_back() function. 
  * std::vector and std::string are good examples for a range.
  */

namespace sfl
{
    /**
      * elem is the list membership predicate, usually written in infix form, e.g., x `elem` xs. 
      */
    template<typename A, typename R>
    bool elem(const A &elem, const R &range)
    {
        auto it = find(range.begin(),range.end(),elem);
        if (it == range.end()) {
            return false;
        }
        return true;
    }

    /**
     * The elemIndex function returns the index of the first element in the given range 
     * which is equal (by operator==) to the query element, or Nothing if there is no such element. 
     */
    template<typename A,typename R>
    Maybe<size_t> elemIndex(const A &elem, const R &range)
    {
    	  auto it = find(range.begin(),range.end(),elem);
    	  if (it == range.end()) {
    	    	return Nothing();
    	  }
    	  return std::distance(range.begin(),it);
    }

    /*
     * Removes one item from a sorted range.
     */
    template<typename R,typename T>
    R sortedMinusOne(const R &range, const T &element)
    {
        auto it = lower_bound(range.begin(),range.end(),element);
        if (it != range.end() && *it == element) {
            auto cr = range;
            cr.erase(cr.begin() + std::distance(range.begin(),it));
            return std::move(cr);
        }
        return range;
    }
}

#endif