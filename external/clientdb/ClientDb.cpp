#include <iostream>
#include <algorithm>
#include "ClientDb.h"
#include "SqliteBackend.h"
#include "sfl/Prelude.h"

namespace ClientDb
{
	using std::cout;

#define method Db::

	static std::map<std::string, std::vector<Index>> groupIndexes(const std::vector<Index> &indexes)
	{
		std::map<std::string, std::vector<Index>> ret;
		for (auto &index : indexes) {
			ret[index.entity()].push_back(index);
		}
		return ret;
	}

	shared_ptr<Db> method open(const std::string &filename, const std::vector<Index> &indexes, const std::function<void(const std::function<void()> &)> &delegateQueue, const std::function<void(const shared_ptr<Db> &)> &callback, const shared_ptr<IBackend> &backend, Comparator comparator)
	{
		auto g = groupIndexes(indexes);
		shared_ptr<Db> self(new Db(filename, groupIndexes(indexes), delegateQueue, backend));

		self->_executor.addTask([self,delegateQueue,callback,filename,g]{
			
			bool success = self->_backend->open(filename, g);
	        delegateQueue([=]{
				callback(success ? self : nullptr);
			});
		});
		return self;
	}

	shared_ptr<Db> method openSync(const std::string &filename, const std::vector<Index> &indexes, const std::function<void(const std::function<void()> &)> &delegateQueue, const shared_ptr<IBackend> &backend, Comparator comparator)
	{
		auto g = groupIndexes(indexes);
		shared_ptr<Db> self(new Db(filename, groupIndexes(indexes), delegateQueue, backend));

		bool success = self->_backend->open(filename, g, comparator);
		if (success) {
			return self;
		}
		return nullptr;
	}

	void method destroy(const std::string &filename, const shared_ptr<IBackend> &backend)
	{
		backend->destroy(filename);
	}

	method Db(const std::string &filename, const std::map<std::string,std::vector<Index>> &indexes, const std::function<void(const std::function<void()> &)> &delegateQueue, const shared_ptr<IBackend> &backend, Comparator comparator) :
		_delegateQueue(delegateQueue),
		_indexes(indexes),
		_backend(backend),
		_comparator(comparator)
	{
	}

	method ~Db()
	{
		_executor.addTaskAndWait([this]{
			_backend.reset();
		});
	}

	void method upsert(const std::string &entity, const Json::Value &object, const std::function<void(const Json::Value &, Json::Value &)> &oldObjectHandler)
	{
		auto self = shared_from_this();
		_executor.addTask([self,this,entity,object,oldObjectHandler]() {

			auto oldObject = _backend->upsert(entity, const_cast<Json::Value &>(object), oldObjectHandler);
			
			Json::FastWriter writer;
			//std::cout << "oldie: " << writer.write(oldObject) << " newie: " << writer.write(object) << std::endl;

			enumerateViews([&](const shared_ptr<IView> &view){
				if (oldObject.isNull()) {
					view->inserted(object);
				} else {
					for (auto &field : object.getMemberNames()) {
						if (object[field] != oldObject[field]) {

							view->updated(oldObject, object);
							break;
						}
					}
				}
			});
		});
	}
	
	void method update(const std::string &entity, const ClientDb::Predicate &predicate, const Json::Value &updatedValues)
	{
		// irst get all objects, update them, then tell the views that they have been updated

		auto self = shared_from_this();
		_executor.addTask([self,this,entity,predicate,updatedValues]{

			auto fetched = _backend->fetch(entity, predicate, SortDescriptor(), Json::Value(), 0, 0, false);
			auto updateKeys = updatedValues.getMemberNames();
			for (auto &f : fetched) {
				for (auto &k : updateKeys) {
					f[k] = updatedValues[k]; 
				}
				upsert(entity, f, std::function<void(const Json::Value &, Json::Value &)>());
			}
		});
	}

	Predicate method predicateForPrimaryIndex(const std::string &entity, const Json::Value &object) const
	{
		auto itIndexes = _indexes.find(entity);
		std::vector<Predicate> components;
		for (auto &field : itIndexes->second.at(0).fields()) {
			components.push_back(Predicate(field, object.get(field,Json::nullValue)));
		}
		Predicate primaryAttributes(Predicate::Operator::And, components);

		return primaryAttributes;
	}

	void method remove(const std::string &entity, const Json::Value &object)
	{
		auto self = shared_from_this();
		_executor.addTask([self,this,entity,object]{

			auto primaryAttributes = predicateForPrimaryIndex(entity, object);

			auto oldObjects = _backend->fetch(entity, primaryAttributes, SortDescriptor(std::vector<std::string>(),true), Json::Value(), 1);
			if (oldObjects.empty()) {
				return;
			}
			auto oldObject = oldObjects[0];
			_backend->remove(entity, primaryAttributes);

			enumerateViews([&](const shared_ptr<IView> &view){
				view->deleted(oldObject);
			});
		});
	}

	void method remove(const std::string &entity, const Predicate &predicate)
	{	
		auto self = shared_from_this();
		_executor.addTask([self,this,entity,predicate]{
			_backend->remove(entity, predicate);

			enumerateViews([&](const shared_ptr<IView> &view){
				view->deleted(predicate);
			});
		});
	}

	std::vector<std::string> method remove(const std::string &entity, const ClientDb::Predicate &predicate, 
				const std::string &field, const std::vector<std::string> &values)
	{
		auto self = shared_from_this();
		std::vector<Json::Value> deletedItems;
		_executor.addTaskAndWait([&]{
			deletedItems = _backend->remove(entity,predicate,field,values,0);
			enumerateViews([&](const shared_ptr<IView> &view){
				for (auto &item : deletedItems) {
					view->deleted(item);
				}
			});
		});
		return sfl::map([&](const Json::Value &item){return item.get(field,"").asString();}, deletedItems);
	}

	void method enumerateViews(const std::function<void(const shared_ptr<IView> &)> &f)
	{
		// must be called from db thread
		_views.erase(remove_if(_views.begin(), _views.end(), [&](const weak_ptr<IView> &w){
			auto view = w.lock();
			if (view) {
				f(view);
				return false;
			} else {
				// remove views not used anymore
				return true;
			}
		}), _views.end());
	}	

	/*void method findOne(const std::string &entity, const Json::Value &query, const function<void(const Json::Value &)> &result)
	{
		auto self = shared_from_this();
		_executor.addTask([self,this,entity,query,result]{
			auto res =_backend->findOne(entity, query);
			_delegateQueue([res,result]{
				result(res);
			});
		});
	}
*/
	void method size(const std::string &entity, const Predicate &predicate, const std::function<void(size_t)> &immediateResult)
	{
		auto self = shared_from_this();
		_executor.addTask([self,this,entity,predicate,immediateResult]{
			auto res = _backend->count(entity, predicate, 0);
			immediateResult(res);
		});
	}

	uint32_t method count(const std::string &entity, const Predicate &predicate, int connectionId)
	{
		auto self = shared_from_this();
#ifdef SERIALIZED_DATABASE
		return _backend->count(entity, predicate, connectionId);
#else
		uint32_t ret;

		_executor.addTaskAndWait([&]{
			ret = _backend->count(entity, predicate, 0);
		});
		return ret;
#endif
	}

	int32_t method max(const std::string &entity, const Predicate &predicate, const std::string &field)
	{
		auto self = shared_from_this();
		int32_t ret;
		_executor.addTaskAndWait([&]{
			ret = _backend->max(entity, predicate, field);
		});
		return ret;
	}

	size_t method indexOf(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const Json::Value &item)
	{
		std::vector<Json::Value> values = fetchSync(entity, predicateForPrimaryIndex(entity, item), SortDescriptor(), 1, 0, false);
		if (values.empty()) {
			return -1;
		}
		return count(entity, Predicate::compositeAnd(predicate, Predicate::lessThan(orderBy, values.at(0))), 0);
	}

	std::vector<Json::Value> method fetchSync(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, size_t limit, size_t offset, bool waitForChanges)
	{
		
#ifdef SERIALIZED_DATABASE
		if (waitForChanges) {
			std::vector<Json::Value> ret;
			_executor.addTaskAndWait([&]{
				ret = _backend->fetch(entity, predicate, orderBy, Json::nullValue, limit, offset);
			});
			return ret;
		} else {
			return _backend->fetch(entity, predicate, orderBy, Json::nullValue, limit, offset);
		}
#else
		std::vector<Json::Value> ret;
		_executor.addTaskAndWait([&]{
			ret = _backend->fetch(entity, predicate, orderBy, Json::nullValue, limit, offset);
		});
		return ret;
#endif
	}

	std::vector<std::string> method fetchSync(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const std::string &field)
	{
#ifdef SERIALIZED_DATABASE
		return _backend->fetch(entity, predicate, orderBy, field);
#else
		std::vector<std::string> ret;
		_executor.addTaskAndWait([&]{
			ret = _backend->fetch(entity, predicate, orderBy, field);
		});
		return ret;
#endif
	}	

	std::vector<Json::Value> method fetchGroupings(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy,
											const std::vector<std::string> &fetchFields, const std::vector<std::string> &groupBy)
	{
#ifdef SERIALIZED_DATABASE
		return _backend->fetchGroupings(entity, predicate, orderBy, fetchFields, groupBy);
#else
#error deprecated
#endif
	}



	void method freeUpMemory()
	{
		_backend->freeUpMemory(1);
        // there is no reason why we should wait for this, and might cause lock!
        auto backend = _backend;
		_executor.addTask([backend]{
			backend->freeUpMemory(0);
		});
	}
}