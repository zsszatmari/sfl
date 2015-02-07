#ifndef CHROMECASTAPPLE_H
#define CHROMECASTAPPLE_H

#include "stdplus.h"
#include <functional>

using std::function;

namespace Cast 
{
	class CastDevice;

	class ChromeCastApple final
	{
	public:
		void probeAsync(const function<void(const MEMORY_NS::shared_ptr<CastDevice> &)> &callback);

	private:
		function<void(const MEMORY_NS::shared_ptr<CastDevice> &)> _callback;
	};
}

#endif