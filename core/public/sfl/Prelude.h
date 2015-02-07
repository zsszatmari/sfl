#ifndef SFL_PRELUDE_H
#define SFL_PRELUDE_H

#include <vector>
#include <functional>
#include <type_traits>
#include "Maybe.h"


/**
  * General purpose functions. Note the terminology 'range' refers to a genericcontainer. Requirements vary,
  * but it must have a begin() and end() iterators, and many times a push_back() function. 
  * std::vector and std::string are good examples for a range.
  */
namespace sfl
{
    /**
     * map(f, xs) is the list obtained by applying f to each element of xs
     */
    template<typename F,typename R,typename A = typename R::value_type,typename B = typename std::result_of<F(A &&)>::type>
    std::vector<B> map(F &&f, const R &range)
    {
        std::vector<B> ret;
        ret.reserve(std::distance(range.begin(),range.end()));
        transform(range.begin(), range.end(), back_inserter(ret), f);
        return std::move(ret);
    }

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
    
    /**
     * Extract the first component of a pair.
     */
    template<typename A,typename B>
    A fst(const std::pair<A,B> &p)
    {
    	  return p.first;
    }

    /**
     * Extract the second component of a pair.
     */
    template<typename A,typename B>
    B snd(const std::pair<A,B> &p)
    {
    	  return p.second;
    }

    /**
      * Append two ranges, 
      * i.e., {x1, ..., xm} + {y1, ..., yn} == {x1, ..., xm, y1, ..., yn] 
      */
    template<typename R>
    R operator+(const R &lhs, const R &rhs)
    {
        R ret;
        ret.reserve(std::distance(lhs.begin(),lhs.end()) + std::distance(rhs.begin(),rhs.end()));
        std::copy(lhs.begin(),lhs.end(),back_inserter(ret));
        std::copy(rhs.begin(),rhs.end(),back_inserter(ret));
        return std::move(ret);
    }

    /** 
      * Adds ane element to the front of the range.
      */
    template<typename R,typename T = typename R::value_type, typename M = typename R::iterator>
    R cons(const T &lhs, const R &rhs)
    {
        R ret;
        ret.reserve(std::distance(rhs.begin(),rhs.end()) + 1);
        ret.push_back(lhs);
        std::copy(rhs.begin(),rhs.end(),back_inserter(ret));
        return std::move(ret);
    }

    /**
     * O(n) Append an element to the end of a range.
     */
    template<typename R,typename T = typename R::value_type>
    R snoc(const R &lhs, const T &rhs)
    {
        R ret;
        ret.reserve(std::distance(lhs.begin(),lhs.end()) + 1);
        std::copy(lhs.begin(),lhs.end(),back_inserter(ret));
        ret.push_back(rhs);
        return std::move(ret);
    }

    /**
     * O(n^2) Append multiple elements to the end of a range.
     */
    template<typename R,typename T,typename... Ts>
    R snoc(const R &lhs, const T &rhs, const Ts &... others)
    {
        return snoc(snoc(lhs,rhs),others...);
    }

    /**
      * filter, applied to a predicate and a range, returns the range of those elements 
      * that satisfy the predicate
      */
    template<typename F,typename R,typename T = typename R::value_type>
    R filter(F &&f, const R &range)
    {
        R ret;
        std::copy_if(range.begin(),range.end(),back_inserter(ret),f);
        return std::move(ret);
    }

    /**
      * The partition function takes a predicate a list and returns the pair of lists of 
      * elements which do and do not satisfy the predicate, respectively.
      */
    template<typename F,typename R>
    std::pair<R,R> partition(F &&f, const R &range)
    {
        std::pair<R,R> ret;
        std::partition_copy(range.begin(),range.end(),back_inserter(ret.first),back_inserter(ret.second),f);
        return std::move(ret);
    }

    /*
     * The sort function implements a sorting algorithm. It is not guaranteed to be stable. 
     */
    template<typename R>
    R sort(const R &range)
    {
        R ret(range.begin(),range.end());
        std::sort(ret.begin(), ret.end());
        return ret;
    }

    /*
     * O(n*log(n)). The ordNub function removes duplicate elements from a list. (The name nub means `essence'.)
     * It guarantees to be sorted, that means not retaining the original order.
     */
    template<typename R>
    R ordNub(const R &range)
    {
        R ret(range.begin(),range.end());
        sort(ret.begin(), ret.end());
        ret.erase(std::unique(ret.begin(),ret.end()),ret.end());
        return ret;
    }

    /**
      * The intersperse function takes an element and a list and `intersperses' that element
      * between the elements of the list. For example, intersperse(',',"abcde") == "a,b,c,d,e" 
      */
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

    /**
      * Concatenate a list of lists.
      */
    template<typename I, typename R = typename I::value_type>
    R concat(const I &range)
    {
        R ret;
        for (const R &element : range) {
            copy(element.begin(), element.end(), back_inserter(ret));
        }
        return std::move(ret);
    }

    /** 
      * intercalate(xs,xss) is equivalent to concat(intersperse(xs,xss)). It inserts the range 
      * xs in between the lists in xss and concatenates the result.
      */
    template<typename T, typename R>
    T intercalate(const T &innerElement, const R &r)
    {
        return concat(intersperse(innerElement, r));
    }

    /**
      * length returns the length of a range.
      */
    template<typename R>
    size_t length(const R &r)
    {
        return std::distance(r.begin(),r.end());
    }

    /** 
      * Extract the first element of a range, which must be non-empty.
      */
    template<typename R,typename T = typename R::value_type>
    T head(const R &r)
    {
        return *r.begin();
    }

    /**
      * Extract the last element of a list, which must be non-empty.
      */
    template<typename R>
    R last(const R &range)
    {
        return *(range.end()-1);
    }

    /**
      * Extract the elements after the head of a list, which must be non-empty.
      */
    template<typename R>
    R tail(const R &r)
    {
        return R(r.begin()+1, r.end());
    }

    /** 
      * take(n,xs), applied to a range xs, returns the prefix of xs of length n, or xs itself 
      * if n > length x
      */
    template<typename R>
    R take(size_t n, const R &r)
    {
        return n < length(r)
                ? R(r.begin(), r.begin()+n) 
                : R(r.begin(), r.end());
    }

    /**
      * drop(n,xs) returns the suffix of xs after the first n elements, or empty range if n > length(xs)
      */
    template<typename R>
    R drop(size_t n, const R &r)
    {
        return n < length(r) 
                ? R(r.begin() + n, r.end())
                : R(r.end(),r.end());
    }
}

#endif
