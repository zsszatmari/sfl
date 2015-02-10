#include <condition_variable>
#include "IoService.h"
#include "BackgroundExecutor.h"

using MEMORY_NS::shared_ptr;

namespace Gear
{
#define method Io::

	static shared_ptr<Io> insta;

	Io & method instance()
	{
		if (!insta) {
			insta = shared_ptr<Io>(new Io());
		}
		return *insta;
	}

	boost::asio::io_service & method get()
	{
		return instance()._io;
	}

	bool method isCurrent()
	{
		return THREAD_NS::this_thread::get_id() == instance()._threadId;
	}

	static THREAD_NS::condition_variable _exitCondition;
	static THREAD_NS::mutex _exitMutex;
	static bool _exited;
	static shared_ptr<boost::asio::io_service::work> _dummyWork;

	void method terminate()
	{
		{
			_exited = false;

			_dummyWork.reset();

			insta->_io.stop();

			THREAD_NS::unique_lock<THREAD_NS::mutex> l(_exitMutex);
			while (!_exited) {
				_exitCondition.wait(l);
			}
		}

		insta.reset();
	}

	method Io() :
		_context(boost::asio::ssl::context::sslv23_client)
	{
		Base::BackgroundExecutor::instance().addTask([this]{

			_threadId = THREAD_NS::this_thread::get_id();

			// don't exit immedietaly because there is no work to do (yet)
			_dummyWork = shared_ptr<boost::asio::io_service::work>(new boost::asio::io_service::work(_io));

			// run indefinitely
			_io.run();
            
			THREAD_NS::lock_guard<THREAD_NS::mutex> l(_exitMutex);
			_exited = true;
			_exitCondition.notify_all();
		});
	}
}