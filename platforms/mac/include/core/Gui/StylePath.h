#ifndef STYLEPATH_H_
#define STYLEPATH_H_

#include <string>
#include <vector>
#include <set>

namespace Gui
{
	class StylePath
	{
	public:
		// like "h3#identifier.class1 li#identifier.class1.class2"
		StylePath(const std::string &path);

		struct Element
		{
			std::string tag;
			std::string identifier;
			std::set<std::string> classes;
		};
		const std::vector<Element> &elements() const;

	private:
		std::vector<Element> _elements;
	};
}

#endif