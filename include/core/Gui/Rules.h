#ifndef STYLERULES_H
#define STYLERULES_H

#include <string>
#include <cstdint>
#include "json.h"
#include "Color.h"
#include "Gradient.h"
#include "sfl/just_ptr.h"

namespace Gui
{
	class IPaintable;
	class Style;

	class Rules
	{
	public:
		Rules();

		struct Spacing
		{
			Spacing();
			Spacing(uint16_t top, uint16_t right, uint16_t bottom, uint16_t left);

			uint16_t top, right, bottom,left;

			bool operator==(const Spacing &rhs) const;
		};

		std::string imageName() const;
		Spacing margin();
		const Json::Value & rules() const;
		Color color() const;
		Gradient gradient() const;
		sfl::just_ptr<IPaintable> paintable() const;

	private:
		Rules(const Json::Value &json);
		Json::Value _rules;

		Spacing spacing(const std::string &prefix) const;

		friend class Style;
	};
}

#endif
