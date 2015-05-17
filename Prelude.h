#ifndef SFL_PRELUDE_H
#define SFL_PRELUDE_H

#include <vector>
#include <functional>
#include <type_traits>
#include <algorithm>

/**
  * General purpose functions. 
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

    /**
      * O(n^2). The nub function removes duplicate elements from a list. In particular, it keeps only the first 
      * occurrence of each element. (The name nub means `essence'.) Original order is preserved.
      */
    template<typename R>
    R nub(const R &range)
    {
        R ret;
        for (auto &element : range) {
            if (find(ret.begin(),ret.end(),element) == ret.end()) {
                ret.push_back(element);
            }
        }
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
      * The intersperse function takes an element and a range and `intersperses' that element
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
      * Maps function f over a list, then concatenates (flattens) the result. F is expected
      * to have a return type of vector<R::value_type>
      */
    template<typename F,typename R,typename A = typename R::value_type,typename B = typename std::result_of<F(A &&)>::type>
    B bind(F &&f, const R &range)
    {
        return concat(map(f, range));
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
    template<typename R,typename T = typename R::value_type>
    T last(const R &range)
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

    /**
     * fold, applied to a binary operator, a starting value (typically the left-identity of the
     * operator), and a list, reduces the list using the binary operator, from left to right.
     */
    template<typename F,typename R,typename A = typename R::value_type,typename B = typename std::result_of<F(A &&)>::type>
    B foldl(F &&f, const B &initial, const R &range)
    {
        auto value = initial;
        for (auto &item : range) {
            value = f(value, item);;
        }
        return value;
    }

    /**
     * foldl, applied to a binary operator, a starting value (typically the left-identity of the
     * operator), and a list, reduces the list using the binary operator, from left to right.
     * This is a recursive function.
     */
    template<typename F,typename R,typename A = typename R::value_type,typename B = typename std::result_of<F(A &&)>::type>
    B foldlR(F &&f, const B &initial, const R &range)
    {
        return range.empty()
                    ? initial
                    : foldlR(f,
                             f(initial,head(range)),
                             tail(range)); 
    }

    /* 
     * takeWhile applied to a predicate p and a range xs, returns the longest prefix (possibly empty) 
     * of xs of elements that satisfy p.
     */
    template<typename F,typename R>
    R takeWhile(F &&p, const R &xs)
    {
        auto it = find_if(xs.begin(),xs.end(),[&p](const typename R::value_type &x){return !p(x);});
        return R(xs.begin(),it);
    }

    /*
     * span, applied to a predicate p and a range xs, returns a tuple where first element is longest prefix 
     * (possibly empty) of xs of elements that satisfy p and second element is the remainder of the list:
     */
    template<typename F,typename R>
    std::pair<R,R> span(F &&p, const R &xs)
    {
        auto it = find_if(xs.begin(),xs.end(),[&p](const typename R::value_type &x){return !p(x);});
        return std::make_pair(R(xs.begin(),it),R(it,xs.end()));
    }

    /**
      * The group function takes a range and returns a vector of ranges such that the concatenation of the 
      * result is equal to the argument. Moreover, each sublist in the result contains only equal (by f) elements. 
      */
    template<typename F,typename R>
    std::vector<R> groupByR(F &&eq, const R &range)
    {
        if (length(range) == 0) {
            return std::vector<R>();
        } else {
            auto x = head(range);
            auto xs = tail(range);
            auto yszs = span(std::bind(eq,x,std::placeholders::_1),xs);
            return cons(cons(x,yszs.first),groupByR(eq,yszs.second));
        }
    }

    /**
      * group by the the default comparison operator
      */
    template<typename R,typename T = typename R::value_type>
    std::vector<R> groupR(const R &range)
    {
        return groupByR([](const T &lhs, const T &rhs){return lhs == rhs;}, range);
    }

    /*
     * replicate(n,x) is a list of length n with x the value of every element. 
    */
    template<typename T>
    std::vector<T> replicate(size_t n, const T &x)
    {
        return std::vector<T>(n,x); 
    }

    /*
     * Generates an arithmetic sequence, with max possibly included. 
     */ 
    inline std::vector<long> sequence(long min, long inc, long max)
    {
        std::vector<long> ret;
        long value = min;
        while (value <= max) {
            ret.push_back(value);
            value += inc;
        }
        return ret;
    }

    /* 
     * zip takes two lists and returns a list of corresponding pairs. If one input list is short, 
     * excess elements of the longer list are discarded.
     */
    template<typename R,typename S,typename Rv = typename R::value_type, typename Sv = typename S::value_type>
    std::vector<std::pair<Rv,Sv>> zip(const R &r, const S &s)
    {
        std::vector<std::pair<Rv,Sv>> ret;
        auto itR = r.begin();
        auto itS = s.begin();
        while (itR != r.end() && itS != s.end()) {
            ret.push_back(std::make_pair(*itR,*itS));
            ++itR;
            ++itS;
        }
        return std::move(ret);
    }

    /*
     * zip3 takes three ranges and returns a range of triples, analogous to zip.
     */
    template<typename R,typename S,typename T, typename Rv = typename R::value_type, typename Sv = typename S::value_type, typename Tv = typename T::value_type>
    std::vector<std::tuple<Rv,Sv,Tv>> zip3(const R &r, const S &s, const T &t)
    {
        std::vector<std::tuple<Rv,Sv,Tv>> ret;
        auto itR = r.begin();
        auto itS = s.begin();
        auto itT = t.begin();
        while (itR != r.end() && itS != s.end() && itT != t.end()) {
            ret.push_back(std::make_tuple(*itR,*itS,*itT));
            ++itR;
            ++itS;
            ++itT;
        }
        return std::move(ret);
    }

    /*
     * Applied to a predicate and a range, all determines if all elements of the range satisfy the predicate. 
     */
    template<typename F,typename R>
    bool all(F &&f, const R &r)
    {
        for (auto &v : r) {
            if (!f(v)) {
                return false;
            } 
        }
        return true;
    }

    /*
     * Applied to a predicate and a range, any determines if any element of the range satisfies the predicate. 
     */
    template<typename F,typename R>
    bool any(F &&f, const R &r)
    {
        for (auto &v :r) {
            if (f(v)) {
                return true;
            }
        }
        return false;
    }
}

#endif
