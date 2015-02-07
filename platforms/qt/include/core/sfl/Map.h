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
}

#endif
