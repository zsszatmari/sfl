#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "RemoteControlServer.h"
#include "RemoteControl.h"
#include "tests.h"

using namespace Gear;
using namespace testing;

class MockDiscover : public RemoteControl::DiscoverDelegate
{
public:
	MOCK_METHOD3(foundRemoteControllable, void(const std::string &, const std::string &, int));
};

TEST(Remote, Discovery)
{
	auto server = RemoteControlServer::create();
	auto client = RemoteControl::create();

	shared_ptr<MockDiscover> disco(new MockDiscover());
	EXPECT_CALL(*disco, foundRemoteControllable(_,_,_)).Times(AtLeast(1));
	client->discover(disco);

	sleep_for(milliseconds(100));
}