#include "stf.h"
#include "sfl/ImmutableVector.h"

using namespace sfl;

TEST(ImmutableVector, Basics)
{
	std::vector<std::string> original = {"a","b","c","d","e"};
	auto immutable = toImmutableVector(original);

	ASSERT_EQ(immutable.size(), 5);

	auto t = take(2, immutable);
	auto d = drop(2, immutable);

	ASSERT_EQ(t.size(), 2);
	ASSERT_EQ(d.size(), 3);

	ASSERT_EQ(t.at(0), "a");
	ASSERT_EQ(t.at(1), "b");
	ASSERT_EQ(d.at(0), "c");
	ASSERT_EQ(d.at(1), "d");
	ASSERT_EQ(d.at(2), "e");

	// similar to what chunk() does, but we also do reassign here! 
	t = take(2, d);
	d = drop(2, d);

	ASSERT_EQ(t.size(), 2);
	ASSERT_EQ(d.size(), 1);

	ASSERT_EQ(t.at(0), "c");
	ASSERT_EQ(t.at(1), "d");
	ASSERT_EQ(d.at(0), "e");

	auto otherRef = immutable;
	ASSERT_EQ(immutable.size(), 5);
}

TEST(ImmutableVector, BasicsShort)
{
	std::vector<std::string> original = {"a","b","c"};
	auto immutable = toImmutableVector(original);

	ASSERT_EQ(immutable.size(), 3);

	auto t = take(2, immutable);
	auto d = drop(2, immutable);

	ASSERT_EQ(t.size(), 2);
	ASSERT_EQ(d.size(), 1);

	ASSERT_EQ(t.at(0), "a");
	ASSERT_EQ(t.at(1), "b");
	ASSERT_EQ(d.at(0), "c");	
}
