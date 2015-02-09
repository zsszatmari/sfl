#ifndef REMOTEPLAYLIST_H
#define REMOTEPLAYLIST_H

#include "IPlaylist.h"

namespace ClientDb
{
	class SortDescriptor;
	class Predicate;
}

namespace Gear
{
	class RemotePlaylist final : public IPlaylist
	{
	public:
		static Json::Value serialize(const vector<shared_ptr<IPlaylist>> &playlists);
		static vector<shared_ptr<IPlaylist>> deserialize(const Json::Value &array);

		virtual const string playlistId() const;
        virtual const string name() const;
        virtual void setName(const string &name);
        virtual const shared_ptr<ISongArray> songArray();
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        virtual bool saveForOfflinePossible() const;

	private:
		RemotePlaylist();

		std::string _playlistId;
		std::string _name;
		shared_ptr<ISongArray> _songArray;
	};
}

#endif