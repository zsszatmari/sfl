#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GooglePlaySessionImpl.h"
#include "FreeSearchArray.h"
#include "FakeApp.h"
#include "AllAccessPlaylist.h"
#include "UnionPlaylist.h"
#include "UnionSongArray.h"
#include "MockSession.h"
#include "tests.h"

using namespace Gear;

TEST(FreeSearch, Artist)
{
	FakeApp::start();

	using namespace testing;

	shared_ptr<MockSession> session(new MockSession());
	auto arr = FreeSearchArray::create(session);

	SongEntry a("src", "all", "a", "a");
	SongEntry b("src", "all", "b", "b");
	auto asong = a.song();
	auto bsong = b.song();
	asong->setStringForKey("artist","monkument");
	asong->setStringForKey("title","onum");
	bsong->setStringForKey("artist","cure");
	bsong->setStringForKey("title","onum");

	EXPECT_EQ("monkument", asong->artist());
	asong->setStringForKey("artist","monument");
	EXPECT_EQ("monument", asong->artist());

	vector<SongEntry> v;
	v.push_back(a);
	v.push_back(b);

	EXPECT_CALL(*session, searchSync(_,_))
	    .Times(4)
	    .WillOnce(DoAll(SetArgReferee<1>("token"),Return(v)))
	    .WillOnce(Return(vector<SongEntry>{}))
	    .WillOnce(DoAll(SetArgReferee<1>("token"),Return(v)))
	    .WillOnce(Return(vector<SongEntry>{}));
	arr->setFilterPredicate(SongPredicate("", "onum", SongPredicate::Contains()));
	sleep_for(milliseconds(20));
	ASSERT_EQ(2, arr->songsReallySlow()->size());

	arr->setFilterPredicate(SongPredicate("artistNorm", "onum", SongPredicate::Contains()));
	sleep_for(milliseconds(20));
	ASSERT_EQ(1, arr->songsReallySlow()->size());
}

TEST(FreeSearch, ArtistByPlaylist)
{
	FakeApp::start();

	shared_ptr<MockSession> session(new MockSession());
	AllAccessPlaylist playlist(session);

	using namespace testing;

	SongEntry a("src", "all", "a", "a");
	SongEntry b("src", "all", "b", "b");
	auto asong = a.song();
	auto bsong = b.song();
	asong->setStringForKey("artist","monument");
	asong->setStringForKey("title","onum");
	bsong->setStringForKey("artist","cure");
	bsong->setStringForKey("title","onum");

	vector<SongEntry> v;
	v.push_back(a);
	v.push_back(b);

	EXPECT_CALL(*session, searchSync(_,_))
	    .Times(4)
	    .WillOnce(DoAll(SetArgReferee<1>("token"),Return(v)))
	    .WillOnce(Return(vector<SongEntry>{}))
	    .WillOnce(DoAll(SetArgReferee<1>("token"),Return(v)))
	    .WillOnce(Return(vector<SongEntry>{}));
	playlist.setFilterPredicate(SongPredicate("", "onum", SongPredicate::Contains()));
	sleep_for(milliseconds(20));
	ASSERT_EQ(2, playlist.songArray()->songsReallySlow()->size());

	playlist.setFilterPredicate(SongPredicate("artistNorm", "onum", SongPredicate::Contains()));
	sleep_for(milliseconds(20));
	ASSERT_EQ(1, playlist.songArray()->songsReallySlow()->size());
}

TEST(FreeSearch, ArtistByUnion)
{
	FakeApp::start();

	shared_ptr<MockSession> session(new MockSession());
	auto allaccess = FreeSearchArray::create(session);
	auto array = UnionSongArray::create();

	vector<shared_ptr<SortedSongArray>> arrays;
	arrays.push_back(allaccess);
	array->setSongArrays(arrays);

	using namespace testing;

	SongEntry a("src", "all", "a", "a");
	SongEntry b("src", "all", "b", "b");
	auto asong = a.song();
	auto bsong = b.song();
	asong->setStringForKey("artist","monument");
	asong->setStringForKey("title","onum");
	bsong->setStringForKey("artist","cure");
	bsong->setStringForKey("title","onum");

	vector<SongEntry> v;
	v.push_back(a);
	v.push_back(b);

	EXPECT_CALL(*session, searchSync(_,_))
	    .Times(4)
	//    .WillOnce(DoAll(SetArgReferee<1>("token"),Return(v)))
	//    .WillOnce(Return(vector<SongEntry>{}))
	    .WillOnce(DoAll(SetArgReferee<1>("token"),Return(v)))
	    .WillOnce(Return(vector<SongEntry>{}))
	    .WillOnce(DoAll(SetArgReferee<1>("token"),Return(v)))
	    .WillOnce(Return(vector<SongEntry>{}));
	array->setFilterPredicate(SongPredicate("", "onum", SongPredicate::Contains()));
	sleep_for(milliseconds(20));
	EXPECT_EQ(2, array->songsReallySlow()->size());

	array->setFilterPredicate(SongPredicate("artistNorm", "onum", SongPredicate::Contains()));
	sleep_for(milliseconds(20));
	EXPECT_EQ(1, array->songsReallySlow()->size());

	testing::Mock::VerifyAndClearExpectations(session.get());
	sleep_for(milliseconds(20));
}

// didn't manage this one to work, might signal a problem:
/*
TEST(FreeSearch, ArtistByUnionPlaylist)
{
	FakeApp::start();

	shared_ptr<MockSession> session(new MockSession());
	shared_ptr<AllAccessPlaylist> allaccess(new AllAccessPlaylist(session));
	UnionPlaylist playlist("","");
	vector<shared_ptr<IPlaylist>> playlists;
	playlists.push_back(allaccess);
	playlist.setPlaylists(playlists);

	using namespace testing;

	shared_ptr<PlayableSong> asong(new PlayableSong("a", session));
	shared_ptr<PlayableSong> bsong(new PlayableSong("b", session));
	asong->setStringForKey("artist","monument");
	asong->setStringForKey("title","onum");
	bsong->setStringForKey("artist","cure");
	bsong->setStringForKey("title","onum");

	SongEntry a(asong,"a");
	SongEntry b(bsong,"b");
	vector<SongEntry> v;
	v.push_back(a);
	v.push_back(b);

	EXPECT_CALL(*session, searchSync(_,_))
	    .Times(4)
	    .WillOnce(DoAll(SetArgReferee<1>("token"),Return(v)))
	    .WillOnce(Return(vector<SongEntry>{}))
	    .WillOnce(DoAll(SetArgReferee<1>("token"),Return(v)))
	    .WillOnce(Return(vector<SongEntry>{}));
	playlist.setFilterPredicate(SongPredicate("", "onum", SongPredicate::Contains()));
	sleep_for(milliseconds(20));
	EXPECT_EQ(2, playlist.songArray()->songs()->size());

	playlist.setFilterPredicate(SongPredicate("artistNorm", "onum", SongPredicate::Contains()));
	sleep_for(milliseconds(20));
	EXPECT_EQ(1, playlist.songArray()->songs()->size());
}
*/
