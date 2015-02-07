#ifndef SORTDESCRIPTOR_H
#define SORTDESCRIPTOR_H

#include <vector>
#include <string>
#include "jsoncpp/json-forwards.h"

namespace ClientDb
{
	class SortDescriptor final
	{
	public: 
		SortDescriptor();
		SortDescriptor(const std::vector<std::string> &orderBy, bool ascending);
		int compare(const Json::Value &lhs, const Json::Value &rhs) const;

		const std::vector<std::string> &orderBy() const;
		bool ascending() const;

	private:
		std::vector<std::string> _orderBy;
		bool _ascending;
	};
}

#endif
