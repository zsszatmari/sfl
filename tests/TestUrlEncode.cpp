#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "UrlEncode.h"

using namespace Gear;
using namespace testing;

TEST(UrlEncode, DontEncodeAmpersand)
{
	// encode '&'. not encoding it makes last.fm not work with artists whose names are containing it
	// the same goes for '='
	// why wouldn't we do this anyway?

	std::string original("abc123=-&");
	std::string expected("abc123%3D-%26");
	auto encoded = UrlEncode::encode(original);
	EXPECT_EQ(expected, encoded);
	EXPECT_EQ(original, UrlEncode::decode(encoded));
}