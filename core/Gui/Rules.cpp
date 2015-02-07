#include "Rules.h"
#include "NamedImage.h"

namespace Gui
{

#define method Rules::

	method Rules()
	{
	}

	method Rules(const Json::Value &json) :
		_rules(json)
	{
	}

	MEMORY_NS::shared_ptr<IPaintable> method paintable() const
	{
		auto image = imageName();
		if (!image.empty()) {
			return MEMORY_NS::shared_ptr<IPaintable>(new NamedImage(image));
		}
		return nullptr;
	}

	std::string method imageName() const
	{
		return _rules.get("image","").asString();
	}


	Rules::Spacing method margin()
	{
		return spacing("margin-");
	}

	Rules::Spacing method spacing(const std::string &prefix) const
	{
		return Spacing(
			_rules.get(prefix+"top", 0).asUInt(),
			_rules.get(prefix+"right", 0).asUInt(),
			_rules.get(prefix+"bottom", 0).asUInt(),
			_rules.get(prefix+"left", 0).asUInt());
	}

	const Json::Value & method rules() const
	{
		return _rules;
	}	

#undef method

#define method Rules::Spacing::

	method Spacing() :
		top(0), right(0), bottom(0), left(0)
	{
	}
	
	method Spacing(uint16_t atop, uint16_t aright, uint16_t abottom, uint16_t aleft) :
		top(atop), right(aright), bottom(abottom), left(aleft)
	{
	}

	bool method operator==(const Spacing &rhs) const
	{
		return top == rhs.top && right == rhs.right && bottom == rhs.bottom && left == rhs.left;
	}

#undef method
}