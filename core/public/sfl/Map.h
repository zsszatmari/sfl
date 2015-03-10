#ifndef SFL_MAP_H
#define SFL_MAP_H

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
         * O(n*log n). Build a map from a list of key/value pairs. If the list contains more than one value for the range
         * same key, the last value for the key is retained.
         */
        template<typename R,typename M = typename std::map<typename R::value_type::first_type,typename R::value_type::second_type>>
        M fromRange(const R &r)
        {
            return M(r.begin(),r.end());
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
    }
}

#endif
