#ifndef VIEW_H
#define VIEW_H

#include <string>
#include <map>
#include <set>
#include <functional>
#include <algorithm>
#include "stdplus.h"
#include MEMORY_H
#include SHAREDFROMTHIS_H
#include ATOMIC_H
#include "json-forwards.h"
#include "Index.h"
#include "Predicate.h"
#include <iostream>
#include <cassert>
#include "View.h"
#include "json.h"
#include MUTEX_H
#include CONDITION_VARIABLE_H
#include "sfl/Prelude.h"
using MEMORY_NS::shared_ptr;
using MEMORY_NS::weak_ptr;

namespace ClientDb
{
	class Db;
	using std::function;

	class IView : public MEMORY_NS::enable_shared_from_this<IView>
	{
	public:
		virtual ~IView() {}

		enum class Event
		{
			Inserted = 0,
			Deleted = 1,
			Updated = 2,
			Reload = 3
		};

	public:
		virtual void inserted(const Json::Value &object) = 0;
		virtual void updated(const Json::Value &oldObject, const Json::Value &object) = 0;
		virtual void deleted(const Json::Value &oldObject) = 0;
		virtual void deleted(const Predicate &predicate) = 0;

	protected:
		static uint32_t fetchCount(const shared_ptr<Db> &db, const std::string &entity, const Predicate &predicate, int connectionId);
		static std::vector<Json::Value> fetch(const shared_ptr<Db> &db, const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const Json::Value &justBefore, size_t limit, size_t offset = 0, int connectionId = 0);
	};

	template<class T>
	class View : public IView
	{
	public:
		// by default callbacks are executed on the db delegate queue but one can easily forward them to app main thread if desired
		// also the methods are thread safe, and non-blocking

		T at(size_t index) const;
		void setVisibleRange(size_t offset, size_t size);
		size_t size() const;

		void fetchRange(size_t pos, size_t size);
		
	private:
		View(const std::string &entity, const std::function<void(const std::function<void()> &)> &viewQueue, const std::function<void(const shared_ptr<View> &,Event,size_t,size_t)> &callback, const shared_ptr<Db> &db, const SortDescriptor &orderBy, const Predicate &predicate, size_t prefetchSize = 32);

		View(const View &); // delete
		View &operator=(const View &); // delete

		shared_ptr<View<T>> shared_from_this() const;

		// called from ClientDb
		void inserted(const Json::Value &object);
		void updated(const Json::Value &oldObject, const Json::Value &object);
		void deleted(const Json::Value &oldObject);
		void deleted(const Predicate &predicate);
		
		//int findObject(const Json::Value &object);
		void queueSync(const std::function<void()> &f) const;

		class FetchState
		{
		public:
			// we must invalidate the vsibileoffset/visiblesize values accordingly, when the fetched values are updated
			FetchState();

			T at(size_t pos) const;
			int offset() const;
			int size() const;
			void clear();
			void set(int offset, const std::vector<T> &v);
			void insert(int offset, const Json::Value &value);
			size_t insert(const function<bool(const T &,const T &)> &comp, const T &value);
			size_t remove(const function<bool(const T &,const T &)> &comp, const T &value);
			void incrementOffset(int value);
			T first() const;
			T last() const;

		private:
			std::vector<T> _fetchedValues;
			ATOMIC_NS::atomic_int _visibleOffset;
			ATOMIC_NS::atomic_int _visibleSize;
		};
		mutable FetchState _fetchState;
		mutable int _size;
		//mutable mutex _m;

		const int _prefetchSize;
		const std::string _entity;
		const std::function<void(const std::function<void()> &)> _queue;
		const std::function<void(const shared_ptr<View> &,Event,size_t,size_t)> _callback;
		const weak_ptr<Db> _db;
		const SortDescriptor _orderBy;
		const Predicate _predicate;

		friend class Db;
	};

#define def template<class T>
#define method View<T>::

	using THREAD_NS::mutex;
	using THREAD_NS::condition_variable;
	using THREAD_NS::lock_guard;
	using THREAD_NS::unique_lock;
	using std::function;

	def
	View<T>::FetchState::FetchState()
	{
		_visibleOffset = 0;
		_visibleSize = 0;
	}

	def
	int View<T>::FetchState::offset() const
	{
		return _visibleOffset;
	}

	def
	int View<T>::FetchState::size() const
	{
		return _visibleSize;
	}

	def
	void View<T>::FetchState::clear()
	{
		set(0,std::vector<T>());
	}

	def
	void View<T>::FetchState::set(int offset, const std::vector<T> &v)
	{
		_visibleSize = 0;
		_fetchedValues = v;
		_visibleOffset = offset;
		_visibleSize = v.size();
	}

	def
	void View<T>::FetchState::insert(int offset, const Json::Value &value)
	{
		if (offset >= _visibleOffset && offset <= (_visibleOffset + _visibleSize)) {
			_fetchedValues.insert(_fetchedValues.begin() + (offset - _visibleOffset), value);
			++_visibleSize; 
		}
	}

	def
	void View<T>::FetchState::incrementOffset(int value)
	{
		_visibleOffset += value;
	}

	def
	T View<T>::FetchState::first() const
	{
		if (_visibleSize > 0) {
            try {
                return _fetchedValues.at(0);
            } catch (...) {
                throw std::runtime_error("no first");
            }
        } else {
			return T();
		}
	}

	def
	T View<T>::FetchState::last() const
	{
		if (_visibleSize > 0) {
            try {
                return _fetchedValues.at(_visibleSize-1);
            } catch (...) {
                throw std::runtime_error("no last");
            }
		} else {
			return T();
		}
	}

	def
	T View<T>::FetchState::at(size_t pos) const
	{
		if (pos < _visibleOffset || pos >= (_visibleOffset + _visibleSize)) {
			return T();
		}
		return _fetchedValues.at(pos - _visibleOffset);
	}

	def
	size_t View<T>::FetchState::insert(const function<bool(const T &,const T &)> &comp, const T &value)
	{	
		auto it = lower_bound(_fetchedValues.begin(), _fetchedValues.end(), value, comp);
		auto pos = std::distance(_fetchedValues.begin(), it);
        _fetchedValues.insert(it, value);
        ++_visibleSize;
        return pos;
	}

	def
	size_t View<T>::FetchState::remove(const function<bool(const T &,const T &)> &comp, const T &value)
	{
		auto it = lower_bound(_fetchedValues.begin(), _fetchedValues.end(), value, comp);
		auto pos = std::distance(_fetchedValues.begin(), it);
        if (it != _fetchedValues.end()) {
            --_visibleSize;
            _fetchedValues.erase(it);
		}
		return pos;
	}
	
	def
	method View(const std::string &entity, const function<void(const function<void()> &)> &viewQueue, const function<void(const shared_ptr<View> &,Event,size_t,size_t)> &callback, const shared_ptr<Db> &db, const SortDescriptor &orderBy, const Predicate &predicate, size_t prefetchSize) :
		_entity(entity),
		_callback(callback),
		_db(db),
		_prefetchSize(prefetchSize),
		_size(-1),
		_orderBy(orderBy),
		_queue(viewQueue),
		_predicate(predicate)
	{
	}

	static size_t cacheLimit()
	{
		return 100;
	}

	def
	shared_ptr<View<T>> method shared_from_this() const
	{
		return MEMORY_NS::static_pointer_cast<View<T>>(MEMORY_NS::const_pointer_cast<IView>(IView::shared_from_this()));
	}

	def
	size_t method size() const
	{
		/*if (_size < 0) {
			auto db = _db.lock();
			if (db) {

				auto self = MEMORY_NS::const_pointer_cast<View>(shared_from_this());
				db->size(_entity, _predicate, [self,this](size_t size){
					_queue([self, this, size]{
						//std::cout << "found " << i << " items\n";

						// only if still uninitialized
						if (_size < 0) {
                            
                            std::cout << "computing size: " << size << std::endl;
							self->_size = size;
							if (size > 0) {
								self->_callback(self,Event::Inserted, 0, size);
							}
						}
					});
				});
			}

			return 0;
		}
		return _size;*/
		if (_size < 0) {
			auto db = _db.lock();
			int size = 0;
			if (db) {
				size = fetchCount(db, _entity, _predicate, 1);
				//size = db->count(_entity, _predicate, 1);
				std::cout << "computing size: " << size << std::endl;
			}
			_size = size;
			if (size > 0) {
				_callback(shared_from_this(),Event::Inserted, 0, size);
			}
		}
		return _size;
	}

	def
	void method setVisibleRange(size_t offset, size_t size)
	{
#ifdef DEBUG
		//std::cout << "visible range: " << offset << " - " << (offset+size-1) << std::endl;
#endif
		const int kRangeGranularity = 500;
		//const int kRangeGranularity = 100;
		auto minmax = std::make_pair(offset, offset+size);
		auto extended = std::make_pair( minmax.first / kRangeGranularity * kRangeGranularity, 
									   (minmax.second + kRangeGranularity-1)/kRangeGranularity *kRangeGranularity );
		offset = extended.first;
		size = extended.second - extended.first;

		if (offset >= _fetchState.offset() && (offset+size) <= (_fetchState.offset() + _fetchState.size())) {
#ifdef DEBUG
            //std::cout << "visible range dontneedupdate: " << offset << " - " << (offset+size-1) << " already have: " << _visibleOffset << "-" << (_visibleOffset + _visibleSize -1) << std::endl;
#endif
			return;
		}

		//_fetchState.set(offset,size);

		auto db = _db.lock();
		if (db) {
			// doing this in background would make this a no-optimization,
			// because we would be forced to get rows one by one

			int justBeforeIndex = -1;
			Json::Value justBefore;

	        if (_orderBy.orderBy().empty()) {
	            throw std::runtime_error("must have an order by statement which is unambigous otherwise it won't return the needed items");
	        }
    
			auto results = fetch(db, _entity, _predicate, _orderBy, justBefore, size, offset, 1);

            if (!results.empty()) {
                int start = justBeforeIndex + 1 + offset;
                int i = start;
                
#ifdef DEBUG
				//std::cout << "visible range extended: " << offset << " - " << (offset+size-1) << " with result: " << start << "-" << (start+results.size()-1)<< std::endl;
#endif
                _fetchState.set(start, sfl::map([](const Json::Value &val){return T(val);},results));
                
                //std::cout << "got results: " << start << " - " << i << " for: " << offset << " - " 
                //	<< (offset + size) << std::endl;

                // calling the callback would be a mistake here, and possibly lead to infinite recursion
                //self->_callback(self,Event::Updated, start, i - start);
            }
		}	
	}

	def
	T method at(const size_t index) const
	{
		auto got = _fetchState.at(index);
		if (!got) {
			auto db = _db.lock();
			if (!db) {
				return Json::Value();
			}
			// 1: the main thread connection
			std::vector<Json::Value> results;
#ifdef DEBUG
            std::cout << "fetching row " << index << " maybe you've forgotten to call setVisibleRange?" << std::endl;
#endif
			if (index < _size/2) {
				results = fetch(db, _entity, _predicate, _orderBy, Json::Value(), 1, index, 1);
			} else {
				SortDescriptor reversed(_orderBy.orderBy(), !_orderBy.ascending());
				results = fetch(db, _entity, _predicate, reversed, Json::Value(), 1, _size-index-1, 1);
			}
			if (results.empty()) {
				return Json::Value();
			} else {
				const auto &result = results.at(0);
				_fetchState.insert(index,result);
				return result;
			}
		}
        return std::move(got);
	}
#if 0
	int method findObject(const Json::Value &object)
	{
		auto db = _db.lock();
		if (!db) {
			return -1;
		}
		int found = -1;
		int lastKnownIndex = -1;
		Json::Value lastKnown;
        
#warning TODO: this makes inserting very very slow, should remove this entire method
		/*queueSync([&]{
			for (auto &p : _fetchedValues) {
				auto &fetchedObj = p.second;
				auto compareResult = _orderBy.compare(object,fetchedObj);
				if (compareResult == 0) {
					found = p.first;
					return;
				} 
				if (compareResult < 0) {
					// object < fetchedobj
					break;
				} else {
					// object > fetchedobj
					lastKnownIndex = p.first;
					lastKnown = fetchedObj;
				}
			}
		});*/
		if (found >= 0) {
			return found;
		}
        
		if (lastKnownIndex >= 0) {
			return lastKnownIndex + db->_backend->count(_entity, Predicate::compositeAnd(_predicate, Predicate::greaterThan(_orderBy, lastKnown), Predicate::lessThan(_orderBy, object)));
		} else {
			return db->_backend->count(_entity, Predicate::compositeAnd(_predicate, Predicate::lessThan(_orderBy, object)));
		}
	}
#endif

	def
	void method inserted(const Json::Value &object)
	{
		if (!_predicate.evaluate(object)) {
			return;
		}

		auto self = shared_from_this();
		_queue([self,this,object]{

			if (self->_size < 0) {
				// not yet intialized, let's trigger it then...
				self->size();
				return;
			}
       
       		self->_size += 1;
       		T obj = object;

			if (_fetchState.size() == 0) {
				self->_callback(self,Event::Inserted, 0, 1); 
			} else if (_orderBy.compare(_fetchState.last(), obj) < 0) {
				// last visible element < new object
				self->_callback(self,Event::Inserted, self->_size-1, 1); 

			} else if (_orderBy.compare(_fetchState.first(), obj) > 0) {
				// first visible element > new object
				_fetchState.incrementOffset(1);
				self->_callback(self,Event::Inserted, 0, 1); 

			} else {

				auto insertHere = _fetchState.insert([&](const T &lhs,const T &rhs){
					return _orderBy.compare(lhs, rhs) < 0;
				}, obj);
                
				self->_callback(self,Event::Inserted, insertHere, 1); 
			}
		});
	}

	def
	void method deleted(const Json::Value &object)
	{
		//std::cout << "deleting...?\n";

		if (!_predicate.evaluate(object)) {
			return;
		}

		//std::cout << "deleting...\n";


		auto self = shared_from_this();
		_queue([self,this,object]{
	
			//std::cout << "deleting... 0\n";

			if (self->_size < 0) {
				// not yet intialized, let's trigger it then...
				//std::cout << "deleting... 1\n";
				self->size();
				self->_callback(self, Event::Reload, 0, 0);

				return;
			}
            
			self->_size -= 1;
			T obj = object;

			if (_fetchState.size() == 0) {

				self->_callback(self,Event::Deleted, 0, 1); 
			} else if (_orderBy.compare(_fetchState.last(), obj) < 0) {

				self->_callback(self,Event::Deleted, self->_size-1, 1); 

			} else if (_orderBy.compare(_fetchState.first(), obj) > 0) {
				_fetchState.incrementOffset(-1);
				self->_callback(self,Event::Deleted, 0, 1); 

			} else {
				auto deleteHere = _fetchState.remove([&](const T &lhs,const T &rhs){
					return _orderBy.compare(lhs, rhs) < 0;
				}, obj);

				if (deleteHere >= 0) {
					self->_callback(self,Event::Deleted, deleteHere, 1); 
				} else {
					self->_callback(self, Event::Updated, _fetchState.offset(), _fetchState.size());
				}
			}
		});
	}

	def
	void method deleted(const Predicate &deletePredicate)
	{
		if (_predicate.subsetOf(deletePredicate)) {
			auto self = shared_from_this();
			_queue([self,this]{
                
				auto oldSize = self->_size;
				self->_size = 0;
				self->_fetchState.clear();
				self->_callback(self, Event::Deleted, 0, oldSize);
			});
		}
	}

	def
	void method updated(const Json::Value &oldObject, const Json::Value &object)
	{
#ifdef DEBUG 
		//assert(oldObject.get("link","") == object.get("link",""));
#endif 

		bool oldEval = _predicate.evaluate(oldObject);
		bool newEval = _predicate.evaluate(object);
		if (!oldEval && !newEval) {
			return;
		}
		if (!oldEval && newEval) {
			inserted(object);
			return;
		}
		if (oldEval && !newEval) {
			deleted(oldObject);
			return;
		}

		// determine if relevant key has changed
		auto comp = _orderBy.compare((T)oldObject, (T)object);
		bool sortRelevantChanged = (comp != 0);

		if (!sortRelevantChanged) {
			auto self = shared_from_this();
			_queue([self,this,object]{
				if (self->_size < 0) {
					// not yet intialized, let's trigger it then...
					self->size();
					return;
				}

				self->_callback(self,Event::Updated, _fetchState.offset(), _fetchState.size());

				// don't overcomplicate, little to gain:
				/*
				int possiblyUpdateHere = -1;

				for (auto &p : self->_fetchedValues) {

					if (_orderBy.compare(p.second, object) >= 0) {
						//newMap[p.first] = p.second;
						updateHere = p.first;
						break;
					}
				}

				// we cannot be sure that this is the exact same object...
				self->_fetchedValues.clear();
				self->_callback(self,Event::Updated, possiblyUpdateHere, 1);*/
			});
		} else {

			//a computing size 1 a logban ide tartozik? es ha igen, mikor lett 0? a deleted alltol bizonyara!
			//std::cout << "sortrelevant update: " << _size << std::endl;
            Json::FastWriter writer;
            //std::cout << "old: " << writer.write(oldObject) << " new: " << writer.write(object) << std::endl;
			//most mi van ha 0 volt a size?
			deleted(oldObject);
			inserted(object);
		}
	}

	def
	void method queueSync(const function<void()> &f) const
	{
		mutex m;
		condition_variable cond;
		bool finished = false;
		_queue([&]{
			f();
			
			lock_guard<mutex> l(m);
			finished = true;
			cond.notify_one();
		});

		{
			unique_lock<mutex> l(m);
			while (!finished) {
				cond.wait(l);
			}
		}
	}

#undef method
#undef def
}

#endif