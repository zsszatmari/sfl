#include "Timer.h"
#include "IoService.h"

namespace Base
{
#define method Timer::

	method Timer() :
		_repeat(false)
#ifndef __APPLE__
		,_timer(Gear::Io::get(), boost::posix_time::microseconds(0))
#endif
	{
#ifdef __APPLE__
		_timer = 0;
#endif
	}

	method Timer(const function<void(Timer *)> &f, float interval, bool repeat, float tolerance) :
		_repeat(repeat)
#ifndef __APPLE__
		,_timer(Gear::Io::get(), boost::posix_time::microseconds(interval * 1000000))
#endif
	{
#ifdef __APPLE__
		function<void(Timer *)> retainedF = f;
		_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0));
        if (repeat) {
            dispatch_source_set_timer(_timer, dispatch_walltime(NULL, 0), interval * NSEC_PER_SEC, tolerance * NSEC_PER_SEC);
        } else {
            dispatch_source_set_timer(_timer, dispatch_time(DISPATCH_TIME_NOW, interval * NSEC_PER_SEC), DISPATCH_TIME_FOREVER, tolerance * NSEC_PER_SEC);
        }
		dispatch_source_set_event_handler(_timer, ^{
            retainedF(this);
        });
        dispatch_resume(_timer);
#else
		_timerF = [f, repeat, interval, this](boost::system::error_code ec){
			if (ec) {
				return;
			}
			f(this);
			if (repeat) {
#pragma message ("TODO: make timer repeat")
                //_timer.expires_at(_timer.expires_at() + boost::posix_time::microseconds(interval * 1000000));
                //_timer.async_wait(this->_timerF);
			}
		};
		_timer.async_wait(_timerF);
#endif
	}

	Timer & method operator=(Timer &&rhs)
	{
		invalidate();
		_repeat = rhs._repeat;
#ifdef __APPLE__
		_timer = rhs._timer;
		rhs._timer = 0;
#else
#endif
		return *this;	
	}

	void method invalidate()
	{
#ifdef __APPLE__
		//if (_repeat) {
		if (_timer) {
			dispatch_release(_timer);
            _timer = 0;
		}
		//}
#else
		_timer.cancel();
#endif	
	}

	method ~Timer()
	{
		invalidate();
	}
}
