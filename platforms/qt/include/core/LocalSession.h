/*
 * LocalSession.h
 *
 *  Created on: Nov 6, 2013
 *      Author: zsszatmari
 */

#ifndef LOCALSESSION_H_
#define LOCALSESSION_H_

#include "SongManipulationSession.h"
#include SHAREDFROMTHIS_H

namespace Gear {

	class ModifiablePlaylist;

	class LocalSession : public SongManipulationSession, public MEMORY_NS::enable_shared_from_this<LocalSession>
	{
	public:
		virtual void init(const shared_ptr<LocalSession> &session);

		virtual void refresh() = 0;

		virtual vector<SongEntry> addSongsToPlaylistSync(const string &playlistId, const vector<shared_ptr<ISong>> &songs) = 0;

		virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs);
		virtual shared_ptr<IPlaybackData> playbackDataSync(const ISong &song) const;
		virtual ValidPtr<const vector<PlaylistCategory>> categories() const;

		virtual void removePlaylist(const shared_ptr<IPlaylist> &playlist);
		virtual void modifySongs(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts);
		virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId) = 0;
		virtual void changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before);
		virtual void changePlaylistNameSync(const string &newName, const string &playlistId);

	protected:
		LocalSession(const shared_ptr<IApp> &app);

		shared_ptr<ModifiablePlaylist> _favorites;
		AtomicPtr<vector<shared_ptr<ModifiablePlaylist>>> _userPlaylists;

	private:
		virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<ISong> &song) const;
		virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<IPlaylist> &song) const;

		virtual shared_ptr<ModifiablePlaylist> createUserPlaylistSync(const vector<shared_ptr<ISong>> &songs, const string &name) = 0;
		virtual void modifySongsSync(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts) = 0;
	};

} /* namespace Gear */
#endif /* LOCALSESSION_H_ */
