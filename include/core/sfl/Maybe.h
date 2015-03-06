#ifndef SFL_MAYBE_H
#define SFL_MAYBE_H

#include <functional>
#include "boost/mpl/at.hpp"
#include "sfl/sum.h"
#include "sfl/Prelude.h"

namespace sfl
{
	class Nothing
	{
	};

	template<typename A>
	using Maybe = boost::variant<Nothing, A>;

	/*
	 * The maybe function takes a default value, a function, and a Maybe value. If the Maybe value is Nothing, 
	 * the function returns the default value. Otherwise, it applies the function to the value inside and returns 
	 * the result.
	 */
	template<typename MaybeType,typename A,typename M = typename boost::mpl::at_c<typename MaybeType::types,1>::type,typename R = typename std::common_type<M,A>::type>
	R maybe(const A &defaultValue, const MaybeType &m)
	{
		return match<M>(m, [&](const Nothing &){return defaultValue;},
						   [](const M &a){return a;});
	}

	/*
	 * The isNothing function returns True iff its argument is Nothing.
	 */
	template<typename MaybeType,typename M = typename boost::mpl::at_c<typename MaybeType::types,1>::type>
	bool isNothing(const MaybeType &m)
	{
		return match<bool>(m, [](const Nothing &){return true;},
						   [](const M &){return false;});
	}

	/*
	 * The isJust function returns True iff its argument has a vslue.
	 */
	template<typename MaybeType,typename M = typename boost::mpl::at_c<typename MaybeType::types,1>::type>
	bool isJust(const MaybeType &m)
	{
		return match<bool>(m, [](const Nothing &){return false;},
						   [](const M &){return true;});
	}

	/**
	  * Maps the f function over the Maybe, resulting in another Maybe which is just f(x) if m wasn't Nothing,
	  * otherwise Nothing.
	  */
	template<typename F,typename MaybeType,typename A = typename boost::mpl::at_c<typename MaybeType::types,1>::type,typename B = typename std::result_of<F(A)>::type>
	Maybe<B> fmap(F &&f, const MaybeType &m)
	{
		return match<Maybe<B>>(m, [](const Nothing &)->Maybe<B>{return Nothing();},
								  [&](const A &a)->Maybe<B>{return f(a);});
	}

	template<typename R,typename MaybeType = typename R::value_type,typename M = typename boost::mpl::at_c<typename MaybeType::types,1>::type>
	std::vector<M> catMaybes(const R &v)
	{
		std::vector<M> ret;
		ret.reserve(length(v));
		for (const auto &item : v) {
			match<void>(item, [](const Nothing &){},
						[&](const M &m){ret.push_back(m);});
		}
		return std::move(ret);
	}

	template<typename R,typename M = typename R::value_type>
	Maybe<M> rangeToMaybe(const R &r)
	{
		return length(r) == 0 
				 ? Maybe<M>(Nothing())
				 : Maybe<M>(head(r));
	}
}

#endif
