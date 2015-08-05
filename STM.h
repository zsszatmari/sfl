#ifndef SFL_STM_H
#define SFL_STM_H

#include <vector>
#include <thread>
#include "sfl/Range.h"
#include "sfl/Maybe.h"
#include "sfl/Map.h"

namespace sfl
{
	using std::mutex;
	using std::lock_guard;
	using std::thread;
	using std::function;
	using std::vector;

	class STM final
	{
	public:
		static void atomically(const function<void(STM &)> &block);
		void retry();
		~STM();

	private:
		STM();
		STM(const STM &) = delete;
		STM &operator=(const STM &) = delete;

		void acquireLock(std::mutex &);
		void addCommit(const function<void()> &commit, const function<void()> &rollback);

		class CurrentTransaction
		{
		public:
			static void set(STM *stm)
			{
				lock_guard<mutex> l(currentTransactionMutex());
				currentTransaction()[std::this_thread::get_id()] = stm;
			}

			static STM *get()
			{
				lock_guard<mutex> l(currentTransactionMutex());
				// we don't ever want a copy here because of compiler problems:
				//return Map::findWithDefault((STM *)nullptr, std::this_thread::get_id(), currentTransaction());
				const auto &m = currentTransaction();
				auto it = m.find(std::this_thread::get_id());
				return (it == m.end()) ? ((STM *)nullptr) : it->second;   
			}

		private:
			static mutex &currentTransactionMutex()
			{
				static mutex *m = new mutex();
				return *m;
			}

			static std::map<thread::id,STM *> &currentTransaction()
			{
				static std::map<thread::id,STM *> *t = new std::map<thread::id,STM *>();
				return *t;
			}
		};

		std::vector<mutex *> _lockedMutexes;
		vector<std::function<void()>> _commitBlocks;
		vector<std::function<void()>> _rollbackBlocks;

		template<class T>
		friend class TVar;
	};

	template<class T>
	class TVar final
	{
	public:
		TVar() :
			_uncommitted(Nothing())
		{
		}

		TVar(const T &value) :
			_value(value),
			_uncommitted(Nothing())
		{
		}

		operator T() const
		{
			return get();
		}

		T get() const
		{
			T ret;
			STM::atomically([&ret,this](STM &s){
				ret = read(s);
			});
			return ret;
		}

		T read(STM &stm) const
		{
			stm.acquireLock(_mutex);
			return match<T>(_uncommitted,[&](const Nothing &){ return _value; },
						                 [&](const T &val){ return val; }); 
		}

		void write(STM &stm, const T &newValue)
		{
			stm.acquireLock(_mutex);
			_uncommitted = newValue;
			stm.addCommit([this,newValue]{
				_value = newValue;
				_uncommitted = Nothing();
			}, [this]{
				_uncommitted = Nothing();
			});
		}

	private:
		TVar(const TVar &) = delete;
		TVar &operator=(const TVar &) = delete;

		T _value;
		Maybe<T> _uncommitted;
		mutable std::mutex _mutex;
	};

	struct retry_transaction : public std::exception
	{
	public:
		retry_transaction(std::mutex *m) :
			_mutex(m) 
		{
		} 

		std::mutex *_mutex;
	};

	inline STM::STM()
	{
	}

	inline void STM::atomically(const function<void(STM &)> &block)
	{
		struct Finally final {
			Finally(const function<void()> &f) : _f(f)
			{
			}

			~Finally()
			{
				_f();
			}

			function<void()> _f;

		private:
			Finally(const Finally &) = delete;
			Finally &operator=(const Finally &) = delete;
		};
		auto curr = CurrentTransaction::get();
		if (curr) {
			block(*curr);
			return;
		}

		Finally f([]{
			CurrentTransaction::set(nullptr);
		});

		bool done = false;
		std::mutex *_firstLock = nullptr;
		while (!done) {	
			STM stm;
			CurrentTransaction::set(&stm);

			Finally ff([]{
				CurrentTransaction::set(nullptr);
			});
			if (_firstLock) {
				// wait until the mutex in question can be locked
				stm._lockedMutexes.push_back(_firstLock);
				_firstLock->lock();
			}
			try {
				block(stm);
				for (auto &c : stm._commitBlocks) {
					c();
				}
				done = true;
			} catch (const retry_transaction &e) {
				for (auto &c : stm._rollbackBlocks) {
					c();
				}
				_firstLock = e._mutex;
			} 
		}
	}

	inline void STM::acquireLock(std::mutex &m)
	{
		if (!elem(&m, _lockedMutexes)) {
			if (m.try_lock()) {
				_lockedMutexes.push_back(&m);
			} else {
				throw retry_transaction(&m);
			}
		}
	}

	inline void STM::addCommit(const function<void()> &commit, const function<void()> &rollback)
	{
		_commitBlocks.push_back(commit);		
		_rollbackBlocks.push_back(rollback);
	}

	inline void STM::retry()
	{
		throw retry_transaction(nullptr);
	}

	inline STM::~STM()
	{
		for (std::mutex *m : _lockedMutexes) {
			m->unlock();
		}
	}
}

#endif