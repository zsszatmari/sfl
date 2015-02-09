#include "gtest/gtest.h"
#include "Style.h"
#include "Rules.h"
#include "NamedImage.h"
#include "Gradient.h"
#include "Color.h"

using namespace testing;
using namespace Gui;

static std::string kStyle = R"(
		{
			"general": {
                    "topbar":[[255,255,255],[224,224,224]],
                    "bottombar":[[224,224,224],[255,255,255]],
                    "text":[4,5,4]
            },
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
	auto named = dynamic_cast<const NamedImage *>(image.get());
	ASSERT_TRUE(named);
	EXPECT_EQ("knob",named->imageName());
}

TEST(Style, Color)
{
	Style style(kStyle);
	auto rules = style.get("general text");
	EXPECT_EQ(Color(4.0f/255.0f, 5.0f/255.0f, 4.0f/255.0f), rules.color());
}

TEST(Style, Gradient)
{
	Style style(kStyle);
	auto rules = style.get("general topbar");
	EXPECT_EQ(Gradient(Color(255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f),
					   Color(224.0f/255.0f, 224.0f/255.0f, 224.0f/255.0f)), rules.gradient());

	auto paintable = rules.paintable();
	auto casted = dynamic_cast<const Gradient *>(paintable.get());
	ASSERT_TRUE(casted);
	EXPECT_EQ(Gradient(Color(255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f),
					   Color(224.0f/255.0f, 224.0f/255.0f, 224.0f/255.0f)), *casted);
}