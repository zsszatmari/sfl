#ifndef SORTDESCRIPTOR_H
#define SORTDESCRIPTOR_H

#include <vector>
#include <string>
#include "json-forwards.h"
#include "StringCompare.h"

namespace ClientDb
{
	class SortDescriptor final
	{
	public: 
		SortDescriptor();
		SortDescriptor(const std::vector<std::string> &orderBy, bool ascending);
		template<class T>
		int compare(const T &lhs, const T &rhs) const;

		const std::vector<std::string> &orderBy() const;
		bool ascending() const;

	private:
		std::vector<std::string> _orderBy;
		bool _ascending;
	};

#define method SortDescriptor::

	template<class T>
	int method compare(const T &lhs, const T &rhs) const
	{
		for (auto &field : _orderBy) {
            {
				auto l = lhs.stringForKey(field);
				auto r = rhs.stringForKey(field);

				int result = Gear::StringCompare::compare(l.c_str(),r.c_str());
				if (result != 0) {
					return _ascending ? result : -result;
				}
			}
			{
				auto l = lhs.floatForKey(field);
				auto r = rhs.floatForKey(field);
				if (l < r) {
					return -1;
				} else if (l > r) {
					return +1;
				}
			}
		}
		return 0;
	}	

#undef method
}

#endif
