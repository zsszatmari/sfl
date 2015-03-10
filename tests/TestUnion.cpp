#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stdplus.h"
#include "IPlaylist.h"
#include "SortedSongArray.h"
#include "FakeApp.h"
#include "MockSession.h"
#include "SongSortOrder.h"
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
