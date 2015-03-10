#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "async.h"
#include "SessionManager.h"
#include "FakeApp.h"
#include "YouTubeSession.h"
#include "FakeOAuthConnection.h"
#include "IPlaylist.h"

TEST(SessionManager, OnlyOneThumbsDown)
{
	using namespace testing;

	auto app = FakeApp::start();
	auto manager = app->sessionManager();
	shared_ptr<ISession> session(YouTubeSession::create(app));
	manager->addSession(session);


	//manager->recomputeCategories();
	async([&](const function<void()> &done){
		std::this_thread::sleep_for(milliseconds(50));

		//auto categories = manager->categories();
		//EXPECT_EQ(3, categories->size());
		auto category = manager->categoryByTag(kAutoPlaylistsTag);
		auto playlists = category.playlists();
		EXPECT_EQ(1, std::count_if(playlists.begin(), playlists.end(), [](auto playlist){return playlist->name() == "Thumbs Down";}));

		done();
	});
}