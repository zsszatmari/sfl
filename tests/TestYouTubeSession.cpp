#include <fstream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "FakeApp.h"
#include "YouTubeSession.h"
#include "FakeOAuthConnection.h"
#include "IPlaylist.h"
#include "YouTubeDecode.h"
#include "IJavascriptEngine.h"

TEST(YouTubeSession, OnlyOneThumbsDown)
{
	using namespace testing;

	auto app = FakeApp::start();
	shared_ptr<ISession> session(YouTubeSession::create(app, shared_ptr<OAuthConnection>(new FakeOAuthConnection())));
	
	auto category = session->categoryByTag(kAutoPlaylistsTag);
	auto playlists = category.playlists();

	// we don't need no thumbs down as sessionmanager manages this, basically filtering out the library
	EXPECT_EQ(0, std::count_if(playlists.begin(), playlists.end(), [](auto playlist){return playlist->name() == "Thumbs Down";}));
}

TEST(YouTubeSession, DecodeJavascript)
{
	YouTubeDecode decoder;

	auto filename = "../tests/youtubeplayer.js";
	std::ifstream t(filename);
	std::string playerJs((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());

	auto engine = decoder.createEngine(playerJs);
	auto result = engine->execute("decode('B568F7783D90B837868410A8756AB737E3481C6A.8E455302CF2438D5886916AACD8CA69A382DCA8D8FF')");

	EXPECT_EQ("B568F7783D90B837868410A8756AB337E3481C6A.8E455302CF2438F5886916AACD8CA69A28DDCA87", result);
}