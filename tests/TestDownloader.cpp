#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "SimpleServer.h"
#include "async.h"
#include "AppleHttpDownloader.h"
#include "PlaybackData.h"
#include "SeekableDownloader.h"

using namespace Gear;

TEST(HttpDownloader, Simple)
{
#ifndef NON_APPLE
	shared_ptr<SimpleServer> server(new SimpleServer(512));
	server->setLimit(200);

	//std::cout << "server open at: " << server->getPort() << std::endl;
	THREAD_NS::thread t([server]{
		server->run();

	});
	t.detach();

	auto downloader = AppleHttpDownloader::create("http://localhost:" + std::to_string(server->port()));
	downloader->start();
	async([downloader](const function<void()> &done){
		downloader->waitAsync(199, [done]{

			done();
		});
	});

	//EXPECT_TRUE(false);
#endif
}

TEST(SeekableDownloader, Seek)
{
	shared_ptr<SimpleServer> server(new SimpleServer(512));
	server->setLimit(200);

	//std::cout << "server open at: " << server->getPort() << std::endl;
	THREAD_NS::thread t([server]{
		server->run();

	});
	t.detach();

	string url("http://localhost:" + std::to_string(server->port()));
	auto downloader = SeekableDownloader::create(url, map<string,string>());
	downloader->start();

	async([&](const function<void()> &done){
		downloader->waitAsync(199, [done,&downloader,&server]{
			//std::cout << "reached first\n";
			server->setLimit(400);
			downloader->seek(300);
			downloader->waitAsync(399, [&downloader,done]{
				//std::cout << "reached second\n";
				downloader->accessChunk(399, [done](const char *ptr, int available){

					EXPECT_EQ((char)(399 % 256), ptr[0]);
					
					done();
				});
			});
		});
	});
}

TEST(PlaybackData, Seek)
{
	shared_ptr<SimpleServer> server(new SimpleServer(512));
	server->setLimit(200);

	//std::cout << "server open at: " << server->getPort() << std::endl;
	THREAD_NS::thread t([server]{
		server->run();

	});
	t.detach();

	vector<string> urls;
	urls.push_back("http://localhost:" + std::to_string(server->port()));
	PlaybackData downloader(urls, IPlaybackData::Format::Mp3);

	async([&](const function<void()> &done){
		downloader.waitAsync(199, [done,&downloader,&server]{
			//std::cout << "reached first\n";
			server->setLimit(400);
			downloader.seek(300);
			downloader.waitAsync(399, [done]{
				done();
			});
		});
	});
}