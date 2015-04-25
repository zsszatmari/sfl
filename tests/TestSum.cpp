#include "stf.h"
#include "sfl/sum.h"

TEST(sum, Match)
{
	using namespace sfl;
	using std::string;

	sum<int,string> left = 321414;
	auto computedLeft = match<int>(left, [](int v){ return v + 666;},
						            	 [](const string &v){ return v.length() + 666;});

	ASSERT_EQ(computedLeft, 322080);

	sum<int,string> right = string("thing");
	auto computedRight = match<int>(right, [](int v){ return v + 666;},
							               [](const string &v){ return v.length() + 666;});

	ASSERT_EQ(computedRight, 671);
}