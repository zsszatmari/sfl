#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "FakeApp.h"
#include "IPlayer.h"
#include "GooglePlayRadio.h"
#include "QueueSongArray.h"
#include "MockSession.h"
#include "GooglePlayRadioArray.h"
#include "SessionManager.h"
#include "tests.h"

TEST(Player, OneSong)
{
	using namespace testing;

	auto app = FakeApp::start();

	auto queue = QueueSongArray::instance();
	app->player()->repeat() = IPlayer::Repeat::OneSong;

	shared_ptr<MockSession> session(new NiceMock<MockSession>());
	IApp::instance()->sessionManager()->addSession(session);
	SongEntry a(session->sessionIdentifier(), "all", "a", "a");
	auto asong = a.song();
	asong->setStringForKey("artist","monument");
	asong->setStringForKey("title","onum");

	vector<SongEntry> radioEntries;
	radioEntries.push_back(a);

#pragma message("TODO: test one song playback")
	/*
	EXPECT_CALL(*session, radioSongsSync(_))
		.WillRepeatedly(Return(radioEntries));

	auto player = IApp::instance()->player();
	player->repeat() = IPlayer::Repeat::OneSong;

	auto radio = GooglePlayRadio::create("radio name", "123", session);
	auto radioArray = MEMORY_NS::dynamic_pointer_cast<GooglePlayRadioArray>(radio->songArray());
	// induce fetch first
	ASSERT_EQ(0, radioArray->songsReallySlow()->size());
	
	sleep_for(milliseconds(20));
	ASSERT_EQ(1, radioArray->songsReallySlow()->size());
	IApp::instance()->userSelectedPlaylist(radio, true);

	//IApp::instance()->player()->play(radioArray->songs()->at(0));

	sleep_for(milliseconds(20));
	
	auto songs = queue->songsReallySlow();
	auto entry = player->songEntryConnector().value();
	ASSERT_TRUE((bool)entry);
	EXPECT_EQ("onum", (std::string)entry.song()->title());
	EXPECT_LT(10, songs->size());

	IApp::instance()->player()->next();
	sleep_for(milliseconds(20));

	entry = player->songEntryConnector().value();
	ASSERT_TRUE((bool)entry);
	EXPECT_EQ("onum", (std::string)entry.song()->title());	

	IApp::instance()->player()->prev();
	sleep_for(milliseconds(20));

	entry = player->songEntryConnector().value();
	ASSERT_TRUE((bool)entry);
	EXPECT_EQ("onum", (std::string)entry.song()->title());

	Mock::VerifyAndClear(session.get());
	IApp::instance()->sessionManager()->removeSession(session);*/
}
