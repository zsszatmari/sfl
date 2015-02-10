#ifndef STYLE_H
#define STYLE_H

#include <string>
#include "json.h"

namespace Gui
{
	class Rules;

	class Style
	{
	public:
		Style();
		Style(const std::string &style);
		Rules get(const std::string &selector) const;

	private:
		Json::Value _style;
	};
}


#endif
