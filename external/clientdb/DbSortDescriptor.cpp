#include "json.h"
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

	const std::vector<std::string> & method orderBy() const
	{
		return _orderBy;
	}

	bool method ascending() const
	{
		return _ascending;
	}
}