#include "gtest/gtest.h"
#include "Style.h"
#include "Rules.h"
#include "NamedImage.h"

using namespace testing;
using namespace Gui;

static std::string kStyle = R"(
		{
			"topbar": {
				"slider": {
					"knob": {
						"margin-top": 3,
						"image": "knob"
					}
				}
			}
		}
	)";

TEST(Style, Basic)
{
	Style style(kStyle);
	auto rules = style.get("topbar slider knob");
	EXPECT_EQ(Rules::Spacing(3,0,0,0), rules.margin());
	auto image = rules.paintable();
	auto named = MEMORY_NS::dynamic_pointer_cast<NamedImage>(image);
	ASSERT_TRUE((bool)named);
	EXPECT_EQ("knob",named->imageName());
}
