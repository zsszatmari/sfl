#include <fstream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "PlaybackItem.h"
#include "FilePlaybackData.h"
#include "FakeApp.h"
#include "tests.h"

using namespace Gear;
using namespace testing;

TEST(PlaybackItem, VeryShort)
{
	FakeApp::start();

	// the problem was that it did not play Napalm Death - You Suffer

	auto filename = "../tests/fakedata.dat";
	std::ifstream t(filename);
	std::string expected((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());

	// actually, it's 12, but might change
	EXPECT_LT(5, expected.size());

	shared_ptr<IPlaybackData> data(new FilePlaybackData(filename, IPlaybackData::Format::PcmSignedInt16));
	auto item = PlaybackItem::create(data);

	char buf[256];
	auto got = item->getUncompressedDataInto(buf, sizeof(buf));
	
	// first we get nothing, but trigger reading
	
	EXPECT_EQ(0, got);

	sleep_for(milliseconds(20));

	got = item->getUncompressedDataInto(buf, sizeof(buf));
	EXPECT_EQ(expected.size(), got);
}
