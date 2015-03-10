#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "RemoteControlServer.h"
#include "RemoteControlClientConnection.h"
#include "async.h"
#include "FakeApp.h"
#include "MockSession.h"
#include "SessionManager.h"
#include "RemoteControl.h"
#include "tests.h"
#include "PhoneCategoriesTransformer.h"

using namespace testing;
using namespace Gear;

#if 0 
TEST(RemoteControl, Categories)
{
	auto app = FakeApp::start();

	shared_ptr<MockSession> session(new MockSession());
	IApp::instance()->sessionManager()->addSession(session);

	{
		auto playlistCategories = app->sessionManager()->categories();
					
		for (auto &category : *playlistCategories) {
			std::cout << "original desktopcat: " << category.title() << std::endl;
			for (auto &playlist : category.playlists()) {
				std::cout << " playlist: " << playlist->name() << std::endl;
			}
		}

		shared_ptr<PhoneCategoriesTransformer> transformer(new PhoneCategoriesTransformer());
		for (auto &category : transformer->transform(*playlistCategories)) {
			std::cout << "original cat: " << category.title() << std::endl;
		}
	}
	
	auto server = RemoteControlServer::create();

	boost::asio::ip::tcp::endpoint destination;
    destination.port(server->port());
    destination.address(boost::asio::ip::address::from_string("127.0.0.1"));
	RemoteControlClientConnection connection;
    
    async([&](const std::function<void()> &done){
		connection.async_connect(destination, [done,&connection](boost::system::error_code ec){
			if (ec) {
				return;
			}

			connection.async_readline([done,&connection](const std::string &line){
				//std::cout << "got reply: " << line << std::endl;
				EXPECT_EQ("categories {}", line);
				connection.async_readline([done,&connection](const std::string &value){
					EXPECT_LT(2, value.size());
					EXPECT_EQ('[', value[0]);
					EXPECT_EQ(']', value[value.size()-1]);

					Json::Reader parser;
					Json::Value json;
					EXPECT_TRUE(parser.parse(value, json));

					auto playlistCategories = PlaylistCategory::deserialize(json);
					
					shared_ptr<PhoneCategoriesTransformer> transformer(new PhoneCategoriesTransformer());
					for (auto &category : playlistCategories) {
						std::cout << "desktopcat: " << category.title() << std::endl;
						for (auto &playlist : category.playlists()) {
							std::cout << " playlist: " << playlist->name() << std::endl;
						}
					}

					playlistCategories = transformer->transform(playlistCategories);

					// there should be only one 'Artists' category after conversion!
					EXPECT_EQ(1, std::count_if(playlistCategories.begin(),playlistCategories.end(),[](auto &cat){return cat.title() == "Artists";}));
					for (auto &category : playlistCategories) {
						std::cout << "cat: " << category.title() << std::endl;
					}

					done();
				});
			});
			connection.async_write("Gcategories {}\n", [&](boost::system::error_code ec){
			});
		});
	});
}
#endif

/*
TEST(RemoteControl, CategoriesFetch)
{
	auto app = FakeApp::start();
	shared_ptr<MockSession> session(new MockSession());
	IApp::instance()->sessionManager()->addSession(session);

	auto server = RemoteControlServer::create();

	client->start("127.0.0.1", server->port());
	sleep_for(milliseconds(20));

	EXPECT_TRUE(false);
}*/