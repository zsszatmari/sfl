#ifndef SFL_SUM_H
#define SFL_SUM_H

#include "boost/variant.hpp"

namespace sfl
{
	// based on lurscher http://stackoverflow.com/questions/7870498/using-declaration-in-variadic-template/7870614#7870614

	template <typename ReturnType, typename... Ts>
	struct visitor;

	template <typename ReturnType, typename Lambda1, typename... Lambdas>
	struct visitor< ReturnType, Lambda1 , Lambdas...> 
	  : public visitor<ReturnType, Lambdas...>, public Lambda1 {

	    using Lambda1::operator();
	    using visitor< ReturnType , Lambdas...>::operator();
	    visitor(Lambda1 l1, Lambdas... lambdas) 
	      : Lambda1(l1), visitor< ReturnType , Lambdas...> (lambdas...)
	    {}
	};

	template <typename ReturnType, typename Lambda1>
	struct visitor<ReturnType, Lambda1> 
	  : public boost::static_visitor<ReturnType>, public Lambda1 {

	    using Lambda1::operator();
	    visitor(Lambda1 l1) 
	      : boost::static_visitor<ReturnType>(), Lambda1(l1)
	    {}
	};

	template <typename ReturnType>
	struct visitor<ReturnType> 
	  : public boost::static_visitor<ReturnType> {

	    visitor() : boost::static_visitor<ReturnType>() {}
	};
    
	template <typename R, typename V, typename... Fs>
	R match(const V &v, Fs... f)
	{
		return boost::apply_visitor(visitor<R,Fs...>(f...), v);
	}
}

#endif