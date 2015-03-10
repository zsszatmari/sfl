/*
 * LocalSession.cpp
 *
 *  Created on: Nov 6, 2013
 *      Author: zsszatmari
 */

#include "LocalSession.h"
#include "NamedImage.h"
#include "FilePlaybackData.h"
#include "ISong.h"

namespace Gear
{
	using std::map; 

#define method LocalSession::

	void method init(const shared_ptr<LocalSession> &ret)
	{
		PlaylistCategory all(kAllCategory, kAllTag, true);
		all.setSingularPlaylist(false);
        
        vector<PlaylistCategory> categories;
		categories.push_back(all);

		auto &allCategoryLists = categories.at(0).playlists();
		//auto freeSearch = FreeSearchArray::create(ret);
		//allCategoryLists.push_back(shared_ptr<IPlaylist>(new BasicPlaylist(ret, "Search", "free", shared_ptr<ISongArray>(freeSearch))));

// local session is out of order now...
		/*ret->_favorites = ModifiablePlaylist::create("all", ret);
		ret->_favorites->setName("Favorites");
		ret->_favorites->setOrdered(false);

		allCategoryLists.push_back(ret->_favorites);

		PlaylistCategory playlists = PlaylistCategory(kPlaylistsCategory, kPlaylistsTag);
		categories.push_back(playlists);

		PlaylistCategory autoPlaylists = PlaylistCategory(kAutoPlaylistsCategory, kAutoPlaylistsTag);
		categories.push_back(autoPlaylists);
		
        auto &librarySongArray = _favorites->storedSongArray();
        modifyCategoryByTag(kAutoPlaylistsTag, [&](PlaylistCategory &autoCat){
            
            auto &autoLists = autoCat.playlists();
            autoLists.push_back(PredicatePlaylist::thumbsUpPlaylist(librarySongArray));
            autoLists.push_back(PredicatePlaylist::thumbsDownPlaylist(librarySongArray));
        });
		
        ret->_categories = categories;*/
	}

	method LocalSession(const shared_ptr<IApp> &app) :
		SongManipulationSession(app)
	{
	}

	void method addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs)
	{
	}

	ValidPtr<const vector<PlaylistCategory>> method categories() const
	{
		return _categories;
	}

	void method modifySongs(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts)
	{
		auto pThis = shared_from_this();
		// tizen broken gcc bugfix
		auto localIds = ids;
		auto localChangedStrings = changedStrings;
		auto localChangedInts = changedInts;
		auto task = [pThis, localIds, localChangedStrings, localChangedInts]{
			pThis->modifySongsSync(localIds, localChangedStrings, localChangedInts);

			/*auto &autoLists = pThis->categoryLocalByTag(kAutoPlaylistsTag).playlists();
			for (auto it = autoLists.begin() ; it != autoLists.end() ; ++it) {
				static_pointer_cast<PredicatePlaylist>(*it)->songArray()->resetCache();
			}*/
		};
		_executor.addTask(task);
	}

	void method removePlaylist(const shared_ptr<IPlaylist> &playlist)
	{
	}

	void method changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before)
	{
	}

	void method changePlaylistNameSync(const string &newName, const string &playlistId)
	{
	}

	shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<ISong> &song) const
	{
		return shared_ptr<Gui::IPaintable>(new Gui::NamedImage("favicon-local"));
	}

	shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<IPlaylist> &song) const
	{
		return shared_ptr<Gui::IPaintable>(new Gui::NamedImage("favicon-local"));
	}

    //static
    IPlaybackData::Format format(const string filename){
        // TODO
        return IPlaybackData::Format::Mp3;
    }
    
	shared_ptr<IPlaybackData> method playbackDataSync(const ISong &song) const
	{
		return shared_ptr<FilePlaybackData>(new FilePlaybackData(song.uniqueId(), format(song.uniqueId())));
	}

} /* namespace Gear */
