#include <sstream>
#include "Style.h"
#include "Rules.h"

namespace Gui
{
#define method Style::

	method Style()
	{
	}

	method Style(const std::string &style)
	{
		Json::Reader reader;
		bool success = reader.parse(style, _style);
#ifdef DEBUG
        //std::cout << "parsed: " << success << std::endl;
#endif
	}

	Rules method get(const std::string &selector) const
	{
		std::istringstream ss(selector);
		std::vector<std::string> v;
		{
			std::string s;
			while (getline(ss, s, ' ')) {
				v.push_back(s);
			}
		}

		const Json::Value *current = &_style;
		for (const auto &s : v) {
			current = &((*current)[s]);
			if (current->isNull()) {
				return Rules();
			}
		}
		return Rules(*current);
	}
}
