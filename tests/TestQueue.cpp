#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GooglePlaySessionImpl.h"
#include "Player.h"
#include "QueueSongArray.h"
#include "IPreferences.h"
#include "AppBase.h"
#include "IKeychain.h"
#include <dispatch/dispatch.h>
#include "FakeApp.h"
#include "MockSession.h"
#include "async.h"
#include "BasicPlaylist.h"
#include "tests.h"
#include "IoService.h"
#include "sessionManager.h"

using namespace Gear;

TEST(Queue, RadioMove)
{
	using namespace testing;

	FakeApp::start();

	auto queue = QueueSongArray::instance();
		//});

	SongEntry a("src", "all", "a", "a");
	SongEntry b("src", "all", "b", "b");
	SongEntry c("src", "all", "c", "c");
	auto asong = a.song();
	auto bsong = b.song();
	auto csong = c.song();
	vector<SongEntry> radioEntries;
	radioEntries.push_back(a);
	radioEntries.push_back(b);
	radioEntries.push_back(c);

#pragma message("TODO: test rearranging of radio")
/*
	shared_ptr<MockSession> session(new MockSession());
	EXPECT_CALL(*session, radioSongsSync(_))
		.WillRepeatedly(Return(radioEntries));
		
	auto radio = GooglePlayRadio::create("radio name", "123", session);
	auto radioArray = MEMORY_NS::dynamic_pointer_cast<GooglePlayRadioArray>(radio->songArray());
	
	radioArray->songsReallySlow();
	
	sleep_for(milliseconds(20));

	{
		auto songs = radioArray->songsReallySlow();
		ASSERT_TRUE(songs->size() == 3);
			
		EXPECT_EQ("a", songs->at(0).entryId());
		EXPECT_EQ("b", songs->at(1).entryId());
		EXPECT_EQ("c", songs->at(2).entryId());
		queue->recalculate(radioArray, songs->at(0));
	}
	sleep_for(milliseconds(80));

	if (3 != queue->songsReallySlow()->size()) {
		abort();
	}
	ASSERT_EQ(3, queue->songsReallySlow()->size());
	EXPECT_TRUE(queue->songsReallySlow()->at(0).song()->uniqueId() == "a");
	EXPECT_TRUE(queue->songsReallySlow()->at(1).song()->uniqueId() == "b");
	EXPECT_TRUE(queue->songsReallySlow()->at(2).song()->uniqueId() == "c");

	vector<SongEntry> m;
	m.push_back(radioArray->songsReallySlow()->at(2));
	
	radioArray->moveSongs(m, radioArray->songsReallySlow()->at(0), radioArray->songsReallySlow()->at(1));
	sleep_for(milliseconds(20));
	
	EXPECT_EQ("a", radioArray->songsReallySlow()->at(0).entryId());
	EXPECT_EQ("c", radioArray->songsReallySlow()->at(1).entryId());
	EXPECT_EQ("b", radioArray->songsReallySlow()->at(2).entryId());

	// this must not be necessary to call manually!
	//queue->recalculate();


	//std::cout << "muuu0\n";
	auto songBase = radioArray->songsReallySlow()->at(0);
	sleep_for(milliseconds(20));
	//std::cout << "muuu1\n";
	async([queue,songBase](auto done){

		Io::get().dispatch([done,queue,songBase]{
			EXPECT_EQ("c", queue->whatToPlayNextAfter(songBase, true).song()->uniqueId());
			//std::cout << "muuu2\n";
			sleep_for(milliseconds(20));
			ASSERT_EQ(3, queue->songsReallySlow()->size());
			//std::cout << "muuu3\n";
			EXPECT_EQ("a", queue->songsReallySlow()->at(0).song()->uniqueId());
			//std::cout << "muuu4\n";
			EXPECT_EQ("c", queue->songsReallySlow()->at(1).song()->uniqueId());
			//std::cout << "muuu5\n";
			EXPECT_EQ("b", queue->songsReallySlow()->at(2).song()->uniqueId());

			done();
		});
	});*/
}

#if 0
TEST(Queue, DeletePlaying)
{
	using namespace testing;

	FakeApp::start();

	auto queue = QueueSongArray::instance();
		//});
	std::cout << "app: " << IApp::instance().get() << " db: " << IApp::instance()->db().get() << std::endl;

	shared_ptr<MockSession> session(new NiceMock<MockSession>());
	IApp::instance()->sessionManager()->addSession(session);

	SongEntry a(session->sessionIdentifier(), "all", "a", "a");
	SongEntry b(session->sessionIdentifier(), "all", "b", "b");
	SongEntry c(session->sessionIdentifier(), "all", "c", "c");
	auto asong = a.song();
	auto bsong = b.song();
	auto csong = c.song();

	vector<SongEntry> entries;
	entries.push_back(a);
	entries.push_back(b);
	entries.push_back(c);

	shared_ptr<SortedSongArray> songArray(new NonLocalArray(session, ClientDb::Predicate("playlist","queuetestDeletePlaying")));
	shared_ptr<BasicPlaylist> playlist(new BasicPlaylist(session, "p", "pp", songArray));
	songArray->setAllSongs(entries);
	sleep_for(milliseconds(50));

	//shared_ptr<MockSession> session(new MockSession());
	auto songs = songArray->songsReallySlow();
	ASSERT_EQ(3, songs->size());
		
	EXPECT_TRUE(songs->at(0).entryId() == "a");
	EXPECT_TRUE(songs->at(1).entryId() == "b");
	EXPECT_TRUE(songs->at(2).entryId() == "c");

	IApp::instance()->userSelectedPlaylist(playlist, true);
	IApp::instance()->player()->play(songs->at(0));
	//queue->recalculate(songArray, songs->at(0));
	sleep_for(milliseconds(60));

	ASSERT_EQ(3, queue->songsReallySlow()->size());
	EXPECT_TRUE(queue->songsReallySlow()->at(0).song()->uniqueId() == "a");
	EXPECT_TRUE(queue->songsReallySlow()->at(1).song()->uniqueId() == "b");
	EXPECT_TRUE(queue->songsReallySlow()->at(2).song()->uniqueId() == "c");

	vector<SongEntry> toDelete;
	toDelete.push_back(a);
	queue->willRemoveSongs(toDelete);
	songArray->removeSongs(toDelete);
	sleep_for(milliseconds(40));
	std::cout << "wait over\n";

	songs = songArray->songsReallySlow();
	ASSERT_EQ(2, songs->size());
	EXPECT_EQ("b", songs->at(0).entryId());
	EXPECT_EQ("c", songs->at(1).entryId());

	ASSERT_EQ(2, queue->songsReallySlow()->size());
	// this is good: when deleting currently playing song, playing must jump to next song
	EXPECT_EQ("b", queue->songsReallySlow()->at(0).song()->uniqueId());
	EXPECT_EQ("c", queue->songsReallySlow()->at(1).song()->uniqueId());
}
#endif 

TEST(Queue, RepeatOne)
{
	using namespace testing;

	auto app = FakeApp::start();

	auto queue = QueueSongArray::instance();
	app->player()->repeat() = IPlayer::Repeat::OneSong;

	SongEntry a("src", "all", "a", "a");
	SongEntry b("src", "all", "b", "b");
	SongEntry c("src", "all", "c", "c");
	auto asong = a.song();
	auto bsong = b.song();
	auto csong = c.song();
	vector<SongEntry> radioEntries;
	radioEntries.push_back(a);
	radioEntries.push_back(b);
	radioEntries.push_back(c);

	shared_ptr<MockSession> session(new MockSession());
#pragma message("TODO: test what happens when deleting currently played track")
	/*EXPECT_CALL(*session, radioSongsSync(_))
		.WillRepeatedly(Return(radioEntries));

	auto radio = GooglePlayRadio::create("radio name", "123", session);
	auto radioArray = MEMORY_NS::dynamic_pointer_cast<GooglePlayRadioArray>(radio->songArray());
	radioArray->songsReallySlow();
	
	sleep_for(milliseconds(20));

	// this would need real song data...
	//IApp::instance()->userSelectedPlaylist(radio, true);
	//IApp::instance()->player()->play(radioArray->songsReallySlow()->at(0));
	queue->recalculate(radioArray, radioArray->songsReallySlow()->at(0));
	sleep_for(milliseconds(20));


	auto songs = queue->songsReallySlow();
	ASSERT_LE(3, songs->size());
	EXPECT_LE(10, songs->size());


	//ASSERT_EQ(3, queue->songsReallySlow()->size());
	EXPECT_EQ("a", songs->at(0).song()->uniqueId());
	EXPECT_EQ("a", songs->at(1).song()->uniqueId());
	EXPECT_EQ("a", songs->at(2).song()->uniqueId());

	// this would only work when playback initiated from the player
	app->player()->repeat() = IPlayer::Repeat::Off;
	queue->recalculate(radioArray, radioArray->songsReallySlow()->at(0));
	sleep_for(milliseconds(80));

	songs = queue->songsReallySlow();
	ASSERT_EQ(3, songs->size());

	EXPECT_EQ("a", songs->at(0).song()->uniqueId());
	EXPECT_EQ("b", songs->at(1).song()->uniqueId());
	EXPECT_EQ("c", songs->at(2).song()->uniqueId());

	app->player()->repeat() = IPlayer::Repeat::On;
	queue->recalculate(radioArray, radioArray->songsReallySlow()->at(0));
	sleep_for(milliseconds(80));

	songs = queue->songsReallySlow();
	ASSERT_LT(6, songs->size());

	EXPECT_EQ("b", songs->at(0).song()->uniqueId());
	EXPECT_EQ("c", songs->at(1).song()->uniqueId());
	EXPECT_EQ("a", songs->at(2).song()->uniqueId());
	EXPECT_EQ("b", songs->at(3).song()->uniqueId());
	EXPECT_EQ("c", songs->at(4).song()->uniqueId());
	EXPECT_EQ("a", songs->at(5).song()->uniqueId());*/
}

TEST(Queue, ShuffleNext)
{
	using namespace testing;

	// many songs (from a playlist), shuffle on, repeat off, start playing, then hit next. there should be some preceding songs
	auto app = FakeApp::start();
	auto queue = QueueSongArray::instance();
	app->player()->repeat() = IPlayer::Repeat::Off;
	app->player()->shuffle() = true;


	vector<SongEntry> radioEntries;
	for (int i = 0 ; i < 10 ; ++i) {
		SongEntry a("src","playlist", std::string("e") + std::to_string(i), std::string("u") + std::to_string(i));
		radioEntries.push_back(a);
	}

	shared_ptr<MockSession> session(new MockSession());
#pragma message("TODO: should test shuffle's effect on queue")

/*	EXPECT_CALL(*session, radioSongsSync(_))
		.WillOnce(Return(radioEntries));

	auto radio = GooglePlayRadio::create("radio name", "queueshufflenext", session);
	auto radioArray = MEMORY_NS::dynamic_pointer_cast<GooglePlayRadioArray>(radio->songArray());

	EXPECT_EQ(0, radioArray->songsReallySlow()->size());
	radioArray->songsReallySlow();
	
	sleep_for(milliseconds(20));
	EXPECT_EQ(10, radioArray->songsReallySlow()->size());

	//ASSERT_EQ(radioEntries.size(), radioArray->songsReallySlow()->size());

	radioArray->setNoRepeatBase(radioArray->songsReallySlow()->at(0));
	// there should be no preceding song
	//std::cout << "here it comes...\n";
	async([radioArray,queue](auto done){
		Io::get().dispatch([done,radioArray,queue]{
			EXPECT_EQ(false, (bool)radioArray->nextShuffledSongRelativeTo(radioArray->songsReallySlow()->at(0), false, false));
			queue->recalculate(radioArray, radioArray->songsReallySlow()->at(0));
			sleep_for(milliseconds(20));

			EXPECT_EQ(queue->songsReallySlow()->at(0).song()->uniqueId(), radioArray->songsReallySlow()->at(0).song()->uniqueId());
			ASSERT_EQ(queue->songsReallySlow()->size(), radioArray->songsReallySlow()->size());
			sleep_for(milliseconds(20));

			// beware of this, causes bad results (since queue changes entryId's
			//SongEntry nextSong = QueueSongArray::instance()->whatToPlayNextAfter(queue->songsReallySlow()->at(0), true);
			// instead:
			SongEntry nextSong = QueueSongArray::instance()->whatToPlayNextAfter(radioArray->songsReallySlow()->at(0), true);
			std::cout << "next song to play: " << nextSong.song()->uniqueId() << std::endl;
			EXPECT_TRUE((bool)nextSong);
			//std::cout << "going next song\n";
			queue->recalculate(radioArray, nextSong);
			sleep_for(milliseconds(200));

			auto radioSongs = *radioArray->songsReallySlow();
			auto queueSongs = *queue->songsReallySlow();
			
#if 0
			std::cout << "radio:\n";
			for (auto &song : radioSongs) {
				std::cout << song.song()->uniqueId() << std::endl;
			}
			std::cout << "queue:\n";
			for (auto &song : queueSongs) {
				std::cout << song.song()->uniqueId() << std::endl;
			}
#endif

			ASSERT_EQ(radioSongs.size(), queueSongs.size());
			EXPECT_EQ(nextSong.song()->uniqueId(), queueSongs.at(1).song()->uniqueId());

			done();
		});
	});*/
}
