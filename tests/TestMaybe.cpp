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