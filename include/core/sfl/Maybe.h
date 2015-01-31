#ifndef SFL_MAYBE_H
#define SFL_MAYBE_H

#include <functional>
#include "boost/mpl/at.hpp"
#include "sfl/sum.h"

namespace sfl
{
	class Nothing
	{
	};

	template<typename A>
	using Maybe = boost::variant<Nothing, A>;

	template<typename MaybeType,typename A,typename M = typename boost::mpl::at_c<typename MaybeType::types,1>::type,typename R = typename std::common_type<M,A>::type>
	R maybe(const A &defaultValue, const MaybeType &m)
	{
		return match<M>(m, [&](const Nothing &){return defaultValue;},
						   [](const M &a){return a;});
	}

	template<typename F,typename MaybeType,typename A = typename boost::mpl::at_c<typename MaybeType::types,1>::type,typename B = typename std::result_of<F(A)>::type>
	Maybe<B> fmap(F &&f, const MaybeType &m)
	{
		return match<Maybe<B>>(m, [](const Nothing &)->Maybe<B>{return Nothing();},
								  [&](const A &a)->Maybe<B>{return f(a);});
	}
}

#endif
