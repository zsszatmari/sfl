#ifndef SFL_MAP_H
#define SFL_MAP_H

#include <map>
#include <utility>
#include "Prelude.h"

/**
  * These function operate on maps, typically std::map or std::unordered_map
  */
namespace sfl
{
	/**
	  * Return all keys of the map. keys will be in ascending order if M is an std::map
	  */
    template<typename M,typename A = typename M::key_type,typename B = typename M::mapped_type>
    std::vector<A> keys(const M &m)
    {
        return map(&fst<A,B>, m);
    }

    namespace Map
    {
        /*
         * O(1) A map with a single element
         */
        template<typename K,typename V>
        std::map<K,V> singleton(const K &k, const V &v)
        {
            std::map<K,V> ret;
            ret.insert(std::make_pair(k,v));
            return ret;
        }

        /*
         * O(n*log n). Build a map from a list of key/value pairs. If the list contains more than one value for the range
         * same key, the last value for the key is retained.
         */
        template<typename R,typename M = typename std::map<typename R::value_type::first_type,typename R::value_type::second_type>>
        M fromRange(const R &r)
        {
            return M(r.begin(),r.end());
        }

        /*
         * O(n) in case of std::map. Convert to a list of key/value pairs.
         */
        template<typename M,typename A = typename M::key_type,typename B = typename M::mapped_type,typename R = typename std::vector<std::pair<A,B>>>
        R toVector(const M &m)
        {
            return R(m.begin(),m.end());
        }

         /*
          * Because of c++ limitations, this is just a synonym for fromRange. Semantically,
          * an already ordered list should be passed here.
          */
        template<typename R,typename M = typename std::map<typename R::value_type::first_type,typename R::value_type::second_type>>
        M fromAscRange(const R &r)
        {
            return fromRange(r);
        }

        /*
         * Build a map from a list of key/value pairs with a combining function.
         */
        template<typename R,typename F,typename A = typename R::value_type::first_type,typename B = typename R::value_type::second_type,typename M = typename std::map<A,B>>
        M fromRangeWith(F &&f, const R &r)
        {
            M ret;
            for (auto &p : r) {
                auto itAndSuccess = ret.insert(std::make_pair(p.first,p.second));
                if (!itAndSuccess.second) {
                    B &value = itAndSuccess.first->second;
                    value = f(value, p.second);
                }
            }
            return std::move(ret);
        }

        /*
         * Group pairs based on firsts, and build a map where the keys are the first, and the values are vectors of seconds
         * For example: [(1,"aa"),(1,"cc"),(2,"aa"),(3,"ff"),(3,"gg"),(1,"bb")] becomes a map of 
         * [(1,["bb","cc","aa"]),(2,["aa"]),(3,["gg","ff"])]
         */
        template<typename R,typename A = typename R::value_type::first_type,typename B = typename R::value_type::second_type,typename M = typename std::map<A,typename std::vector<B>>>
        M sortAndGroup(const R &r)
        {
            return fromRangeWith(operator+<std::vector<B>>, map([](const std::pair<A,B> &p){
                return std::make_pair(p.first,std::vector<B>{{p.second}});
            },r));
        }

        /*
         * O(n+m)?. The expression (union t1 t2) takes the left-biased union of t1 and t2. It prefers t1 
         * when duplicate keys are encountered, i.e.
         */
        template<typename M>
        M mapUnion(const M &lhs,const M &rhs)
        {
            auto ret = lhs;
            for (const auto &p : rhs) {
                ret.insert(p);
            }
            return ret;
        }

        /*
         * The union of a range of maps
         */
        template<typename R,typename M = typename R::value_type>
        M unionsR(const R &r)
        {
            return foldlR(mapUnion<M>, M(),r);
        }

        template<typename M,typename A = typename M::key_type,typename B = typename M::mapped_type>
        B findWithDefault(const B &def, const A &key, const M &m)
        {
            auto it = m.find(key);
            return it == m.end() ? def : it->second;
        } 
    }
}

#endif
