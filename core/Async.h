#ifndef ASYNC_H
#define ASYNC_H

#include "stdplus.h"
#include <functional>

namespace Async
{
	using std::function;

	template<class T,class R>
	void forEach(const std::vector<T> &v, const function<void(const T &, R &, const function<void(const R &)> &cont)> &handleElement, const function<void(const R &)> &finish)
	{		
		struct Iterate
		{
			function<void(const Iterate &, int, R)> fn;
		};
		Iterate it;
		it.fn = [v,handleElement,finish](const Iterate &iterate, int i, R result){
			if (i < v.size()) {
			    auto &element = v[i];
			    handleElement(element,result,[iterate,i](const R &result){
			        iterate.fn(iterate, i+1, result);
			    });
			} else {
				finish(result);
			}
		};

		R result;
		it.fn(it, 0, result);
	}

	template<class T>
	void forEach(const std::vector<T> &v, const function<void(const T &, const function<void()> &)> &handleElement, const function<void()> &finish)
	{
		forEach<T,int>(v, [handleElement](const T &element, int &, const function<void(const int &)> &cont){
			handleElement(element, [cont]{
				cont(0);
			});
		}, [finish](const int &r){
			finish();
		});
	}
}

#endif