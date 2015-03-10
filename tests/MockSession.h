#ifndef MOCKSESSION_H
#define MOCKSESSION_H

#include "GooglePlaySessionImpl.h"
#include "FilePlaybackData.h"

using namespace Gear;

class MockSession : public GooglePlaySessionImpl
{
public:
	MockSession() :
		GooglePlaySessionImpl(IApp::instance())
	{	
	}

	//MOCK_METHOD2(searchSync, const vector<SongEntry>(const string &, string &));
	MOCK_CONST_METHOD0(fetchRatedSongs, const shared_ptr<Json::Value>());
	MOCK_METHOD0(librarySongArray, shared_ptr<StoredSongArray>());
	//MOCK_METHOD1(radioSongsSync, const vector<SongEntry>(const string &));
	//MOCK_CONST_METHOD1(playbackDataSync, shared_ptr<IPlaybackData>(const ISong &));
	/*virtual shared_ptr<IPlaybackData> playbackDataSync(const ISong &) const override
	{
		// prevent premature song end
		return shared_ptr<IPlaybackData>(new FilePlaybackData("",IPlaybackData::Format::Mp3));
	}*/
};

#endif