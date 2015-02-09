#include <iostream>
#include <cassert>
#include "View.h"
#include "json.h"
#include "ClientDb.h"
#include MUTEX_H
#include CONDITION_VARIABLE_H

namespace ClientDb
{
#define method View::

	using THREAD_NS::mutex;
	using THREAD_NS::condition_variable;
	using THREAD_NS::lock_guard;
	using THREAD_NS::unique_lock;
	using std::function;
	
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
		_visibleOffset = 0;
		_visibleSize = 0;
	}

	static size_t cacheLimit()
	{
		return 100;
	}

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
				size = db->count(_entity, _predicate, 1);
				std::cout << "computing size: " << size << std::endl;
			}
            auto self = MEMORY_NS::const_pointer_cast<View>(shared_from_this());
			self->_size = size;
			if (size > 0) {
				self->_callback(self,Event::Inserted, 0, size);
			}
		}
		return _size;
	}

	void method setVisibleRange(size_t offset, size_t size)
	{
#ifdef DEBUG
		//std::cout << "visible range: " << offset << " - " << (offset+size-1) << std::endl;
#endif
		const int kRangeGranularity = 500;
		auto minmax = std::make_pair(offset, offset+size);
		auto extended = std::make_pair( minmax.first / kRangeGranularity * kRangeGranularity, 
									   (minmax.second + kRangeGranularity-1)/kRangeGranularity *kRangeGranularity );
		offset = extended.first;
		size = extended.second - extended.first;

		if (offset >= _visibleOffset && (offset+size) <= (_visibleOffset + _visibleSize)) {
			return;
		}

#ifdef DEBUG
		std::cout << "visible range extended: " << offset << " - " << (offset+size-1) << std::endl;
#endif

		_visibleSize = 0;
		_visibleOffset = offset;
		_visibleSize = size;

		auto db = _db.lock();
		if (db) {
			auto self = shared_from_this();

			// doing this in background would make this a no-optimization,
			// because we would be forced to get rows one by one

			int justBeforeIndex = -1;
			Json::Value justBefore;

	        if (_orderBy.orderBy().empty()) {
	            throw std::runtime_error("must have an order by statement which is unambigous otherwise it won't return the needed items");
	        }
    
			auto results = db->_backend->fetch(_entity, _predicate, _orderBy, justBefore, size, offset, 1);

            if (!results.empty()) {
                auto mself = MEMORY_NS::const_pointer_cast<View>(self);
                int start = justBeforeIndex + 1 + offset;
                int i = start;
                if (mself->_fetchedValues.size() > cacheLimit()) {
                	mself->_fetchedValues.clear();
                }
                for (auto &result : results) {
                    if (i >= _size) {
                        break;
                    }
                    mself->_fetchedValues[i] = result;
                    ++i;
                }
                //std::cout << "got results: " << start << " - " << i << " for: " << offset << " - " 
                //	<< (offset + size) << std::endl;

                // calling the callback would be a mistake here, and possibly lead to infinite recursion
                //self->_callback(self,Event::Updated, start, i - start);
            }
		}	
	}

	Json::Value method at(const size_t index) const
	{
		auto it = _fetchedValues.find(index);
		if (it == _fetchedValues.end()) {
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
				results = db->_backend->fetch(_entity, _predicate, _orderBy, Json::Value(), 1, index, 1);
			} else {
				SortDescriptor reversed(_orderBy.orderBy(), !_orderBy.ascending());
				results = db->_backend->fetch(_entity, _predicate, reversed, Json::Value(), 1, _size-index-1, 1);
			}
			if (results.empty()) {
				return Json::Value();
			} else {
				const auto &result = results.at(0);
				_fetchedValues[index] = result;
				return result;
			}
		}
		return it->second;
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

			if (_fetchedValues.empty()) {
				self->_size += 1;
				self->_callback(self,Event::Inserted, 0, 1); 
			} else if (_orderBy.compare(_fetchedValues.rbegin()->second, object) < 0) {
				// last visible element < new object

				self->_size += 1;
				self->_callback(self,Event::Inserted, self->_size-1, 1); 

			} else if (_orderBy.compare(_fetchedValues.begin()->second, object) > 0) {
				// first visible element > new object
				decltype(self->_fetchedValues) newMap;
				for (auto &p : self->_fetchedValues) {
					newMap[p.first+1] = std::move(p.second);
				}
				swap(self->_fetchedValues,newMap);

				self->_size += 1;
				self->_callback(self,Event::Inserted, 0, 1); 

			} else {
                // this causes problems e.g. when repeatedly inserting into the first slot
                /*
				int insertHere = 0;
				for (auto &p : self->_fetchedValues) {

					if (_orderBy.compare(p.second, object) < 0) {
						insertHere = p.first+1;
					} else {
						break;
					} 
				}
				self->_fetchedValues.clear();
				*/
                
                decltype(self->_fetchedValues) newMap;
                int insertHere = 0;
                bool found = false;

                for (auto &p : self->_fetchedValues) {
                    
                    if (found) {
                    	newMap[p.first+1] = std::move(p.second);
                    	continue;
                    }
                    if (_orderBy.compare(p.second, object) < 0) {
	                    newMap[p.first] = std::move(p.second);
                        insertHere = p.first+1;
                    } else {
                    	newMap[p.first] = object;
                    	newMap[p.first+1] = std::move(p.second);
                        found = true;
                    } 
                }
                swap(self->_fetchedValues,newMap);

				self->_size += 1;
				self->_callback(self,Event::Inserted, insertHere, 1); 
			}
		});
	}

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

			if (_fetchedValues.empty()) {

				self->_size -= 1;
				self->_callback(self,Event::Deleted, 0, 1); 
			} else if (_orderBy.compare(_fetchedValues.rbegin()->second, object) < 0) {

				self->_size -= 1;
				self->_callback(self,Event::Deleted, self->_size-1, 1); 

			} else if (_orderBy.compare(_fetchedValues.begin()->second, object) > 0) {
				decltype(self->_fetchedValues) newMap;
				for (auto &p : self->_fetchedValues) {
					newMap[p.first-1] = p.second;
				}
				swap(self->_fetchedValues,newMap);

				self->_size -= 1;
				self->_callback(self,Event::Deleted, 0, 1); 

			} else {
				decltype(self->_fetchedValues) newMap;
				int deleteHere = -1;
				for (auto &p : self->_fetchedValues) {

					if (_orderBy.compare(p.second, object) >= 0) {
						//newMap[p.first] = p.second;
						deleteHere = p.first;
						break;
					}
				}
				if (deleteHere >= 0) {
					self->_fetchedValues.clear();
					self->_size -= 1;
					self->_callback(self,Event::Deleted, deleteHere, 1); 
				} else {
					self->_fetchedValues.clear();
					self->_size -= 1;

					self->_callback(self, Event::Updated, _visibleOffset, _visibleSize);
				}
			}
		});
	}

	void method deleted(const Predicate &deletePredicate)
	{
		if (_predicate.subsetOf(deletePredicate)) {
			auto self = shared_from_this();
			_queue([self]{
				auto oldSize = self->_size;
				self->_size = 0;
				self->_fetchedValues.clear();
				self->_callback(self, Event::Deleted, 0, oldSize);
			});
		}
	}

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
		auto comp = _orderBy.compare(oldObject, object);
		bool sortRelevantChanged = (comp != 0);

		if (!sortRelevantChanged) {
			auto self = shared_from_this();
			_queue([self,this,object]{
				if (self->_size < 0) {
					// not yet intialized, let's trigger it then...
					self->size();
					return;
				}

				self->_callback(self,Event::Updated, _visibleOffset, _visibleSize);

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
}