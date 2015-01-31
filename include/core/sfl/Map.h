#ifndef SFL_MAP_H
#define SFL_MAP_H

#include "Prelude.h"

namespace sfl
{
    template<typename M,typename A = typename M::key_type,typename B = typename M::mapped_type>
    std::vector<A> keys(const M &m)
    {
        return map(&fst<A,B>, m);
    }
}

#endif