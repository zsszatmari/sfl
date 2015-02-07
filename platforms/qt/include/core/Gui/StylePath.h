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

			bool operator==(const Element &rhs) const;
		};

		StylePath(const std::vector<Element> &elements);
		const std::vector<Element> &elements() const;
		bool matches(const StylePath &selector) const;

	    static std::function<void(const std::vector<char> &)> l(const std::function<void(const std::string &)> &ll);

	private:
		std::vector<Element> _elements;
	};
}

#endif