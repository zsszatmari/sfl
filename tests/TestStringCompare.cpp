#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "StringCompare.h"
#include "SongEntry.h"
#include "SongSortOrder.h"
#include "FakeApp.h"
#include "MockSession.h"
#include "tests.h"

using namespace ClientDb;
using namespace Gear;
using namespace testing;

TEST(StringCompare, HungarianCompare)
{

	char str1[] = "axi";
	char str2[] = "árny";

	EXPECT_TRUE(StringCompare::compare(str1, str2) < 0);

	char s1[] = "a";
	char s2[] = "á";
	char s3[] = "b";

	EXPECT_TRUE(StringCompare::compare(s1, s2) < 0);
	EXPECT_TRUE(StringCompare::compare(s2, s3) < 0);
	EXPECT_TRUE(StringCompare::compare(s1, s3) < 0);
}

TEST(StringCompare, JapaneseCompare) 
{
	/*char test[] = "ア";
	EXPECT_TRUE(strlen(test) == 3);*/

//1st: e382a2e382afe382a2e38386e383a9e383aae382a6e383a0
//2nd: e38286e381a3e3818fe3828ae8b7b3e381ade3828be99fb3e6a5bd

	char str1[] = "アクアテラリウム";
	char str2[] = "ゆっくり跳ねる音楽";

	char littleM[] = "m";
	char bigM[] = "M";

	EXPECT_TRUE(StringCompare::compare(str1, str2) < 0);
	EXPECT_TRUE(StringCompare::compare(littleM, str2) < 0);

}

TEST(StringCompare, JapaneseAfter)
{
	char bigM[] = "M";
	char str1[] = "アクアテラリウム";

	EXPECT_TRUE(StringCompare::compare(bigM, str1) < 0);
}

TEST(StringCompare, SortOrderCompare)
{
    auto app = FakeApp::start();
    
	SongEntry a("src", "all", "a", "a");
	SongEntry b("src", "all", "b", "b");
	auto asong = a.song();
	auto bsong = b.song();
	asong->setStringForKey("artist","b");
	asong->setUintForKey("disc",3);
	bsong->setStringForKey("artist","á");
	bsong->setUintForKey("disc",2);

	std::vector<SongEntry> v;
	v.push_back(a);
	v.push_back(b);

	auto comparator = SongSortOrder::comparator(SongSortOrder::sortDescriptor());
	sort(v.begin(),v.end(), comparator);

	EXPECT_EQ(2, v[0].song()->uIntForKey("disc"));
	EXPECT_EQ(3, v[1].song()->uIntForKey("disc"));
}
/*
TEST(StringCompare, DatabaseCompare)
{
	auto app = FakeApp::start();
	shared_ptr<MockSession> session(new NiceMock<MockSession>());
	auto playlist = ModifiablePlaylist::create("fakeplaylist", "Fake Playlist", session);

	SongEntry a(session->sessionIdentifier(), "fakeplaylist", "a", "a");
	SongEntry b(session->sessionIdentifier(), "fakeplaylist", "b", "b");
	auto asong = a.song();
	auto bsong = b.song();
	asong->setStringForKey("artist","b");
	asong->setUintForKey("disc",3);
	bsong->setStringForKey("artist","á");
	//bsong->setStringForKey("artist","a");
	bsong->setUintForKey("disc",2);

	std::vector<SongEntry> v;
	v.push_back(a);
	v.push_back(b);

	vector<SongEntry> songs;
	songs.push_back(a);
	songs.push_back(b);
	asong->save();
	bsong->save();

	sleep_for(milliseconds(80));

	EXPECT_EQ(2, playlist->songArray()->songsReallySlow()->size());
	EXPECT_EQ(2, playlist->songArray()->songsReallySlow()->at(0).song()->uIntForKey("disc"));

	app->terminate();

}*/
