#include "StyleRules.h"

namespace Gui
{
	#define method StyleRules::

	std::string method getRaw(const std::string &key) const
	{
		auto it = _rules.find(key);
		if (it == _rules.end()) {
			return "";
		}
		return it->second;
	}

	void method clear()
	{
		_rules.clear();
	}
		
	void method set(const std::string &key, const std::string &value)
	{
		_rules[key] = value;
	}

	void method add(const StyleRules &rhs)
	{
		for (auto &p : rhs._rules) {
			_rules[p.first] = p.second;
		}
	}
}