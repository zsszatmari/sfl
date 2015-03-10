#ifndef STYLERULES_H
#define STYLERULES_H

#include <string>
#include "stdplus.h"
#include "json.h"

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

		shared_ptr<IPaintable> paintable() const;
		std::string imageName() const;
		// margin is subject to collapsing
		Spacing margin();
		Spacing padding();
		const Json::Value & rules() const;

	private:
		Rules(const Json::Value &json);
		Json::Value _rules;

		Spacing spacing(const std::string &prefix) const;

		friend class Style;
	};
}

#endif
