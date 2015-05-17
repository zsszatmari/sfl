#include "stf.h"
#include "sfl/Split.h"
#include "sfl/ImmutableVector.h"

using namespace sfl;

TEST(Split, chunkR)
{
	std::vector<std::string> original = {"a","b","c","d","e"};
	auto splitted = chunkR(2,original);

	ASSERT_EQ(3, splitted.size());

	ASSERT_EQ(2, splitted.at(0).size());
	ASSERT_EQ(2, splitted.at(1).size());
	ASSERT_EQ(1, splitted.at(2).size());

	ASSERT_EQ(splitted.at(0).at(0), "a");
	ASSERT_EQ(splitted.at(0).at(1), "b");
	ASSERT_EQ(splitted.at(1).at(0), "c");
	ASSERT_EQ(splitted.at(1).at(1), "d");
	ASSERT_EQ(splitted.at(2).at(0), "e");
}

TEST(Split, chunkRImmutable)
{
	std::vector<std::string> original = {"a","b","c","d","e"};
	auto immutable = toImmutableVector(original);
	auto splitted = chunkR(2, immutable);
	
	ASSERT_EQ(3, splitted.size());

	ASSERT_EQ(2, splitted.at(0).size());
	ASSERT_EQ(2, splitted.at(1).size());
	ASSERT_EQ(1, splitted.at(2).size());

	ASSERT_EQ(splitted.at(0).at(0), "a");
	ASSERT_EQ(splitted.at(0).at(1), "b");
	ASSERT_EQ(splitted.at(1).at(0), "c");
	ASSERT_EQ(splitted.at(1).at(1), "d");
	ASSERT_EQ(splitted.at(2).at(0), "e");
}

TEST(Split, On)
{
	using std::vector;
	using std::string;

	ASSERT_EQ(vector<string>({"a","b",".c","","d",""}), splitOn(string(".."), string("a..b...c....d..")));
	ASSERT_EQ(vector<string>({"https:","","i.ytimg.com","vi","uJ_1HMAGb4k","hqdefault.jpg"}), splitOn(string("/"),string("https://i.ytimg.com/vi/uJ_1HMAGb4k/hqdefault.jpg")));
	// this differs from Haskell Split.splitOn behaviour. the point is that there is no crash
	ASSERT_EQ(vector<string>({"abc"}), splitOn(string(""),string("abc")));
	ASSERT_EQ(vector<string>({""}), splitOn(string(","),string("")));
	ASSERT_EQ(vector<string>({"",""}), splitOn(string(","),string(",")));
}
