#include "Rules.h"
#include "NamedImage.h"
#include "sfl/Prelude.h"
#include "sfl/just_ptr.h"

namespace Gui
{
#pragma message("TODO: before there will be user-editable themes we should ensure exception/crash safety. note that rapidjson wouldn't help, it just has assertions, even worse")

#define method Rules::

	method Rules()
	{
	}

	static Json::Value only(const std::string &name, const Json::Value &json)
	{
		Json::Value ret;
		ret[name] = json;
		return ret;
	}

	static bool isColor(const Json::Value &json)
	{
		if (!json.isArray()) {
			return false;
		}
		for (int i = 0 ; i < json.size() ; ++i) {
			if (!json[i].isNumeric()) {
				return false;
			}
		}
		return true;
	}

	static bool isGradient(const Json::Value &json)
	{
		if (!json.isArray()) {
			return false;
		}
		for (int i = 0 ; i < json.size() ; ++i) {
			if (!isColor(json[i])) {
				return false;
			}
		}
		return true;
	}

	method Rules(const Json::Value &json) :
		_rules(json.isString() 
				? only("image",json) 
				: isColor(json)
					? only("color", json)
					: isGradient(json) 
						? only("gradient", json)
						: json
			  ) 
	{
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

	static Color colorFrom(const Json::Value &components)
	{
		if (!components.isArray() || components.size() < 3) {
			return Color();
		} else {
			return Color(components[0].asFloat() / 255.0f,
						 components[1].asFloat() / 255.0f,
						 components[2].asFloat() / 255.0f,
						 components.size() >= 4 
						 	? components[3].asFloat() / 255.0f 
						 	: 1.0f);
		}
	}

	Color method color() const
	{
		auto components = _rules.get("color",Json::arrayValue);
		return colorFrom(components);
	}

	Gradient method gradient() const
	{
		auto gradient = _rules.get("gradient", Json::arrayValue);
		if (!gradient.isArray()) {
			return Gradient();
		}
		std::vector<Color> colors;
		for (int i = 0 ; i < gradient.size() ; ++i) {
			colors.push_back(colorFrom(gradient[i]));
		}
		if (colors.empty()) {
			return Gradient();
		}

		using namespace sfl;
		auto positions = map([&](int num){return num * (1.0f/(colors.size()-1));}, sequence(0,1,colors.size()-1));
		return Gradient(zip(colors,positions));
	}

	sfl::just_ptr<IPaintable> method paintable() const
	{
		using namespace sfl;

		auto name = imageName();
		if (!name.empty()) {
			return make_just<NamedImage>(name);
		}
		auto grad = gradient();
		if (grad) {
			return make_just<Gradient>(grad);
		}
		auto col = color();
		return make_just<Color>(col);
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