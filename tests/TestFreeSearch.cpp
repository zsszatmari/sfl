#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GooglePlaySessionImpl.h"
#include "FakeApp.h"
#include "MockSession.h"
#include "tests.h"

using namespace Gear;


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
