#include <fstream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "FakeApp.h"
#include "YouTubeSession.h"
#include "FakeOAuthConnection.h"
#include "IPlaylist.h"
#include "YouTubeDecode.h"
#include "IJavascriptEngine.h"
#include "YouTubeProtocol.h"

TEST(YouTubeSession, OnlyOneThumbsDown)
{
	using namespace testing;

	auto app = FakeApp::start();
	shared_ptr<ISession> session(YouTubeSession::create(app));
	
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

TEST(YouTubeSession, VideoInfo)
{
	auto app = FakeApp::start();
	
	auto filename = "../tests/videoinfo.txt";
	std::ifstream t(filename);
	std::string videoinfo((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());

	auto result = YouTubeProtocol::videoUrlWeb(videoinfo);
	EXPECT_EQ(result, "http://r8---sn-po8puxa-c0qe.googlevideo.com/videoplayback?ipbits=0&initcwndbps=782500&ratebypass=yes&sver=3&expire=1422670082&mime=video%2Fmp4&dur=822.288&itag=18&pl=19&source=youtube&gcr=hu&fexp=900718%2C907263%2C910132%2C927622%2C930676%2C931343%2C938628%2C939990%2C9405814%2C943917%2C947225%2C948124%2C952302%2C952605%2C952901%2C955301%2C957201%2C958500%2C959701&sparams=dur%2Cgcr%2Cid%2Cinitcwndbps%2Cip%2Cipbits%2Citag%2Cmime%2Cmm%2Cms%2Cmv%2Cpl%2Cratebypass%2Csource%2Cupn%2Cexpire&id=o-AKLrmJeZiWgBCQI3A6Sc-hdwQitpmVretABaK5InKIQS&mm=31&upn=pC_aFDrIyDc&ip=91.83.232.29&key=yt5&ms=au&mt=1422648395&mv=m&signature=D2545B4BD8D0D4484E239E44A6403B28CF25B741.EF9E32BDA3BE0B10D127E13C019C381A4C0539A2");
}