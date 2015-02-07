#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stdplus.h"
#include "IPlaylist.h"
#include "SortedSongArray.h"
#include "UnionSongArray.h"
#include "FakeApp.h"
#include "MockSession.h"
#include "PredicatePlaylist.h"
#include "SongSortOrder.h"
#include "NonLocalArray.h"
#include "SessionManager.h"
#include "tests.h"

#include CHRONO_H
using CHRONO_NS::milliseconds;

using namespace Gear;
using namespace testing;

/*
class TestSongArray : public SortedSongArray
{
public:
	void setRawSongs(const ValidPtr<const vector<SongEntry>> &v)
	{
		SortedSongArray::setRawSongs(v);
	}
};*/

TEST(Union, Refresh)
{
	auto app = FakeApp::start();
	shared_ptr<MockSession> session(new NiceMock<MockSession>());
	app->sessionManager()->addSession(session);

	SongEntry a(session->sessionIdentifier(), "all", "a", "a");
	auto asong = a.song();
	asong->setStringForKey("artist","monument");
	asong->setStringForKey("title","onum");
	asong->setUintForKey("rating", 5);
	
	shared_ptr<SortedSongArray> library(new NonLocalArray(session, ClientDb::Predicate("playlist","UnionRefresh")));

	ValidPtr<vector<SongEntry>> entries(new vector<SongEntry>());
	entries->push_back(a);
	library->setAllSongs(*entries);

	auto thumbsup = PredicatePlaylist::thumbsUpPlaylist(library);

	vector<shared_ptr<SortedSongArray>> innerArrays;
	innerArrays.push_back(MEMORY_NS::dynamic_pointer_cast<SortedSongArray>(thumbsup->songArray()));

///*
	auto inner = UnionSongArray::create();
	inner->setSongArrays(innerArrays);

	vector<shared_ptr<SortedSongArray>> outerArrays;
	outerArrays.push_back(inner);
	//*/
	auto u = UnionSongArray::create();
	//u->setSortDescriptor(SongSortOrder::sortDescriptor("rating"));
	u->setSongArrays(outerArrays);
	//u->setSongArrays(innerArrays);

	sleep_for(milliseconds(20));
	EXPECT_EQ(1, thumbsup->songArray()->songsReallySlow()->size());
	EXPECT_EQ(1, u->songsReallySlow()->size());

	u->setSortDescriptor(SongSortOrder::sortDescriptor("rating"));
	sleep_for(milliseconds(20));


	IApp::instance()->player()->rate(asong, 1);
	sleep_for(milliseconds(20));
	EXPECT_EQ(1, asong->rating());
	thumbsup->songArray()->resetCache();   // there is an 5s delay, we skip it for the sake of testing

	sleep_for(milliseconds(20));
	EXPECT_EQ(0, thumbsup->songArray()->songsReallySlow()->size());
	EXPECT_EQ(0, u->songsReallySlow()->size());

}