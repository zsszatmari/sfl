#include "stf.h"
#include "sfl/Maybe.h"

using namespace sfl;

TEST(Maybe, isJust)
{
	Maybe<int> just = 6;
	ASSERT_TRUE(isJust(just));
	Maybe<int> nada = Nothing();
	ASSERT_FALSE(isJust(nada));
}

TEST(Maybe, isNothing)
{
	Maybe<int> just = 6;
	ASSERT_FALSE(isNothing(just));
	Maybe<int> nada = Nothing();
	ASSERT_TRUE(isNothing(nada));
}

TEST(Maybe, Cat)
{
	std::vector<Maybe<std::string>> v;
	v.push_back(Nothing());
	v.push_back(std::string("something"));
	v.push_back(Nothing());
	v.push_back(std::string("other thing"));
	
	auto c = catMaybes(v);
	ASSERT_EQ(c.size(), 2);
	ASSERT_EQ(c[0], "something");
	ASSERT_EQ(c[1], "other thing");
}