#ifndef MOCKPLAYLIST_H
#define MOCKPLAYLIST_H


#include "IPlaylist.h"
#include "IPlaylistIntent.h"

using namespace Gear;


class MockPlaylistIntent : public IPlaylistIntent
{
public:
	const std::string menuText() const
	{
		return "Delete";
	}

	MOCK_CONST_METHOD0(confirmationText, const std::string());
	MOCK_METHOD0(apply, void());	
};

class MockPlaylist : public IPlaylist
{
public:
	MockPlaylist() : IPlaylist(weak_ptr<ISession>()) {}

	MOCK_CONST_METHOD0(playlistId, const string());
	MOCK_CONST_METHOD0(name, const string());
	MOCK_METHOD1(setName, void(const string &));
	MOCK_METHOD0(songArray, const shared_ptr<ISongArray>());
	MOCK_METHOD1(dragIntentTo, shared_ptr<ISongNoIndexIntent>(const vector<SongEntry> &));
	MOCK_CONST_METHOD0(saveForOfflinePossible, bool());

	vector<shared_ptr<IPlaylistIntent>> playlistIntents()
	{
		vector<shared_ptr<IPlaylistIntent>> ret;
		ret.push_back(shared_ptr<IPlaylistIntent>(new MockPlaylistIntent()));
		return ret;
	}
};

#endif
