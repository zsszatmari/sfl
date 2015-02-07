#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "SongSortOrder.h"
#include "SongEntry.h"

using namespace Gear;

TEST(SortOrder, Disc)
{
	auto descriptor = SongSortOrder::sortDescriptor("disc");
	EXPECT_EQ("disc", descriptor.keysAndAscending()[0].first);

	SongEntry a("src", "all", "a", "a");
	SongEntry b("src", "all", "b", "b");
	auto asong = a.song();
	auto bsong = b.song();
	asong->setStringForKey("artist","aaaaa");
	asong->setStringForKey("title","aaaaa");
	asong->setUintForKey("disc",3);
	bsong->setStringForKey("artist","pppppp");
	bsong->setStringForKey("title","pppppp");
	bsong->setUintForKey("disc",2);

	vector<SongEntry> v;
	v.push_back(a);
	v.push_back(b);

	auto comparator = SongSortOrder::comparator(descriptor);
	sort(v.begin(),v.end(),comparator);

	EXPECT_EQ(2, v[0].song()->uIntForKey("disc"));
	EXPECT_EQ(3, v[1].song()->uIntForKey("disc"));

}