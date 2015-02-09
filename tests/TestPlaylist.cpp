#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stdplus.h"
#include "IPlaylist.h"
#include "MockPlaylist.h"
#include "UnionPlaylistIntent.h"
#include "FakeApp.h"
#include "MockSession.h"
#include "ModifiablePlaylist.h"
#include "tests.h"

using namespace Gear;

TEST(IPlaylist, UnionIntent)
{
	std::vector<shared_ptr<IPlaylist>> playlists;
	shared_ptr<MockPlaylist> a(new MockPlaylist());
	shared_ptr<MockPlaylist> b(new MockPlaylist());
	playlists.push_back(a);
	playlists.push_back(b);

	auto intents = IPlaylist::playlistIntents(playlists);
	EXPECT_EQ(intents.size(), 1);

	EXPECT_TRUE(MEMORY_NS::dynamic_pointer_cast<UnionPlaylistIntent>(intents.at(0)) != nullptr);
	auto text = intents.at(0)->confirmationText();
	string kPostFix = "2 playlists?";
	//std::cout << intents.at(0)->menuText() << std::endl;
	//std::cout << text << std::endl;
	EXPECT_TRUE(text.compare(text.length() - kPostFix.length(), kPostFix.length(), kPostFix) == 0);
}

TEST(Playlist, RemoveSong)
{
	using namespace testing;

	FakeApp::start();

	shared_ptr<MockSession> session(new NiceMock<MockSession>());
	auto playlist = ModifiablePlaylist::create("fakeplaylist", "Fake Playlist", session);

	vector<SongEntry> songs;
	SongEntry a(session->sessionIdentifier(), "fakeplaylist", "a", "a");
	auto asong = a.song();
	songs.push_back(a);
	asong->save();

	sleep_for(milliseconds(80));

	EXPECT_EQ(1, playlist->songArray()->songsReallySlow()->size());
	playlist->removeSongs(songs);

	// we mustn't crash, etc...
	sleep_for(milliseconds(50));
	EXPECT_EQ(0, playlist->songArray()->songsReallySlow()->size());
}