#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <vector>
#include "jsoncpp/json-forwards.h"

namespace ClientDb
{
	class Index final
	{
	public:
		Index();
		Index(const std::string &entity, const std::string &identifier, const std::vector<std::string> &fields, bool fullTextIndex = false);
		Index &operator=(const Index &index);

		explicit operator bool() const;
		const std::string &entity() const;
		const std::vector<std::string> &fields() const;
		bool operator==(const std::vector<std::string> &fields) const;
		bool fullTextIndex() const;

		// these should be removed from here:
		//const std::string &keyPrefix() const;
		//std::string key(const Json::Value &object, bool forSearch = false) const;
	private:
		std::string _entity;
		//std::string _keyPrefix;
		std::vector<std::string> _fields;
		bool _fullTextIndex;
	};
}


#endif