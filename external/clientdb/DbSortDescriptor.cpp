#include "jsoncpp/json.h"
#include "DbSortDescriptor.h"

namespace ClientDb
{
#define method SortDescriptor::

	method SortDescriptor() :
		_ascending(false)
	{
	}

	method SortDescriptor(const std::vector<std::string> &orderBy, bool ascending) :
		_orderBy(orderBy),
		_ascending(ascending)
	{
	}

	int method compare(const Json::Value &lhs, const Json::Value &rhs) const
	{
		for (auto &field : _orderBy) {
			// TODO: this will not work well in the face of accented characters (not the same as in db)
            auto l = lhs.get(field,Json::nullValue);
            auto r = rhs.get(field,Json::nullValue);
			auto result = l.compare(r);
			if (result != 0) {
				return _ascending ? result : -result;
			}
		}
		return 0;
	}	

	const std::vector<std::string> & method orderBy() const
	{
		return _orderBy;
	}

	bool method ascending() const
	{
		return _ascending;
	}
}