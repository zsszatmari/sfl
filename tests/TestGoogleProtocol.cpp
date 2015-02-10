#include <fstream>
#include <iterator>
#include "json.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GoogleMusicProtocol.h"
#include "GoogleMusicConnection.h"

using namespace Gear;
using namespace testing;

class MockConnection : public GoogleMusicConnection
{
public:
	MOCK_METHOD1(callPure, std::string(const std::string &));
};

TEST(GoogleProtocol, ListOfPlaylists)
{
	shared_ptr<MockConnection> connection(new MockConnection());
	shared_ptr<GoogleMusicProtocol> protocol(new GoogleMusicProtocol(connection));


	std::ifstream t("../tests/googlemusiclisten.html");
	std::string listen((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
	EXPECT_CALL(*connection, callPure(_))
		.WillOnce(Return(listen));

	auto playlists = protocol->getListOfPlaylists();
	ASSERT_TRUE((bool)playlists);
	EXPECT_EQ(19, playlists->size());
}

TEST(GoogleProtocol, ListOfPlaylistsOldWay)
{
	// but it sometimes give date in the old way!
	shared_ptr<MockConnection> connection(new MockConnection());
	shared_ptr<GoogleMusicProtocol> protocol(new GoogleMusicProtocol(connection));


	std::ifstream t("../tests/googlemusiclisten-oldway.html");
	std::string listen((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
	EXPECT_CALL(*connection, callPure(_))
		.WillOnce(Return(listen));

	auto playlists = protocol->getListOfPlaylists();
	ASSERT_TRUE((bool)playlists);
	EXPECT_EQ(19, playlists->size());
}

TEST(GoogleProtocol, ListOfPlaylistsNewWay)
{
	// but it sometimes give date in the old way!
	shared_ptr<MockConnection> connection(new MockConnection());
	shared_ptr<GoogleMusicProtocol> protocol(new GoogleMusicProtocol(connection));


	std::ifstream t("../tests/googlemusiclisten-201410.html");
	std::string listen((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
	EXPECT_CALL(*connection, callPure(_))
		.WillOnce(Return(listen));

	auto playlists = protocol->getListOfPlaylists();
	ASSERT_TRUE((bool)playlists);
	EXPECT_EQ(17, playlists->size());
}

TEST(GoogleProtocol, ListOfPlaylistsNewWayRetry)
{
	// but it sometimes give date in the old way!
	shared_ptr<MockConnection> connection(new MockConnection());
	shared_ptr<GoogleMusicProtocol> protocol(new GoogleMusicProtocol(connection));


	std::ifstream t("../tests/googlemusiclisten-201410-2.html");
	std::string listen((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
	EXPECT_CALL(*connection, callPure(_))
		.WillOnce(Return(listen));

	auto playlists = protocol->getListOfPlaylists();
	ASSERT_TRUE((bool)playlists);
	EXPECT_EQ(18, playlists->size());
}