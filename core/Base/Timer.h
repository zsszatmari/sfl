#include "stdplus.h"
#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include "boost/asio.hpp"
#endif
#include <functional>

namespace Base
{
	using std::function;

	class Timer final
	{
	public:
		Timer();
		Timer(const function<void(Timer *)> &f, float interval, bool repeat, float tolerance);
		Timer &operator=(Timer &&);
		~Timer();

	private:
		Timer(const Timer &); // delete
		Timer &operator=(const Timer &);

		void invalidate();

		bool _repeat;
#ifdef __APPLE__
		dispatch_source_t _timer;
#else
		boost::asio::deadline_timer _timer;
		function<void(boost::system::error_code)> _timerF;
#endif
	};
}