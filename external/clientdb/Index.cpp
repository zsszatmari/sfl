#include <sstream>
#include "Index.h"
#include "json.h"

namespace ClientDb
{
#define method Index::

	method Index(const std::string &entity, const std::string &identifier, const std::vector<std::string> &fields, bool fullTextIndex) :
		_entity(entity),
		//_keyPrefix(entity + "/" + identifier + "/"),
		_fields(fields),
		_fullTextIndex(fullTextIndex)
	{
	}

	bool method fullTextIndex() const
	{
		return _fullTextIndex;
	}

	Index & method operator=(const Index &rhs)
	{
		//_keyPrefix = rhs._keyPrefix;
		_fields = rhs._fields;
		_entity = rhs._entity;
		return *this;
	}

	method Index()
	{
	}

 	method operator bool() const
	{
		return !_fields.empty();
	}

	const std::string & method entity() const
	{
		return _entity;
	}

	/*std::string method key(const Json::Value &object, bool forSearch) const
	{
		std::stringstream ss;
		ss << _keyPrefix;
		bool first = true;
		for (auto &f : _fields) {
			if (!first) {
				ss << "/";
			}
			first = false;

			if (object.isMember(f)) {
				ss << object.get(f,"").asString();
			} else {
				if (forSearch) {
					// if we are searching, we need only the beginning of the key
					break;
				}
			}
		
		}
		return ss.str();
	}*/

	bool method operator==(const std::vector<std::string> &fields) const
	{
		return _fields == fields;
	}

	/*const std::string & method keyPrefix() const
	{
		return _keyPrefix;
	}*/

	const std::vector<std::string> & method fields() const
	{
		return _fields;
	}
}