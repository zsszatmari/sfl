#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "FakeApp.h"
#include "MockSession.h"
#include "GooglePlayThumbsArray.h"
#include "GooglePlayThumbsPlaylist.h"
#include "PredicatePlaylist.h"
#include "UnionPlaylist.h"
#include "ModifiablePlaylist.h"
#include "BaseUtility.h"

TEST(Thumbs, Unordered)
{
	using namespace testing;

	FakeApp::start();

	shared_ptr<MockSession> session(new NiceMock<MockSession>());
	{
		shared_ptr<ISongArray> thumbsArray = GooglePlayThumbsArray::create(session);

		EXPECT_EQ(false, thumbsArray->orderedArray());
	}
	
	/*auto category = session->categoryByTag(kAutoPlaylistsTag);
	auto playlists = category.playlists();
	ASSERT_TRUE(playlists.size() > 0);
	auto thumbsPlaylist = playlists[0];
	EXPECT_EQ("Thumbs Up", thumbsPlaylist->name());*/

	auto libraryPlaylist = ModifiablePlaylist::create("all","Library", session);

	EXPECT_CALL(*session, fetchRatedSongs())
		.WillRepeatedly(Return(shared_ptr<Json::Value>(new Json::Value())));
	EXPECT_CALL(*session, librarySongArray())
		.WillRepeatedly(Return(libraryPlaylist->storedSongArray()));

	shared_ptr<IPlaylist> thumbsUpLocal(PredicatePlaylist::thumbsUpPlaylist(session->librarySongArray()));
	auto thumbsUpEphemeral = shared_ptr<GooglePlayThumbsPlaylist>(new GooglePlayThumbsPlaylist(session));
    shared_ptr<UnionPlaylist> unionThumbsUp(new UnionPlaylist(PredicatePlaylist::thumbsUpPredicate().fingerPrint(), thumbsUpLocal->name()));
    shared_ptr<IPlaylist> t[] = {thumbsUpLocal, thumbsUpEphemeral};
    unionThumbsUp->setPlaylists(init<vector<shared_ptr<IPlaylist>>>(t));

    EXPECT_EQ(false, unionThumbsUp->songArray()->orderedArray());
    Mock::VerifyAndClearExpectations(session.get());
    Mock::AllowLeak(session.get());
}
