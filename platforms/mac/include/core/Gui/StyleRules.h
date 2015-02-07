#ifndef STYLERULES_H
#define STYLERULES_H

#include <string>
#include <map>
#include "Color.h"

namespace Gui
{
	class StyleRules
	{
	public:
		void clear();
		std::string getRaw(const std::string &key) const;
		Color getColor(const std::string &key) const;
		float getFloat(const std::string &key) const;

		void set(const std::string &key, const std::string &value);
		void add(const StyleRules &rhs);

	private:
		std::map<std::string, std::string> _rules;
	};
}

#endif
