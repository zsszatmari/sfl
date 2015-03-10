#ifndef DBSESSION_H
#define DBSESSION_H

#include <memory>
#include "SongManipulationSession.h"

namespace Gear
{
	using std::string;
	using std::shared_ptr;

	class DbSession : public SongManipulationSession, public MEMORY_NS::enable_shared_from_this<DbSession>
	{
	public:
		struct Behaviour
		{
			// an unique identifier, like 'gplay'
			string sourceIdentifier;
			// readable name, like 'Google Play'
			string name;
			// icon filename prefix
			string icon;
			function<void(const string &,const string &,const function<void(bool)> &)> connect;
			/* a function which is called once when logged in, and once when the user explicitly presses
			 * the refresh button. The function then asynchronously fetches data, and passes it to the designated
			 * callback. When all items are fetched, the callback must be called with third argument set to true
			 * to signal end of items,
			 * one for Song, one for Playlist.
			 * If the callback returns false, session is closed, don't call callback anymore.
			 * first argument is either "Song" or "Playlist". The allowed fields for items are described elsewhere.
			 */ 
			typedef function<bool(const string &,const vector<Json::Value> &,bool)> FetchCallback;
			typedef function<void(const FetchCallback &)> FetchLibrary;
			FetchLibrary fetchLibrary;
			function<Json::Value()> presence;

			/*
			 * Should call callback with the right playbackdata object, or nullptr if the song can't be played
			 */
			function<void(const string &,const function<void(const shared_ptr<IPlaybackData> &)> &)> playback;
			// need to pass context for first argument, 'user' field can be abused. context is returned for further work
			// if it is null, then don't continue search
			typedef function<Json::Value(const Json::Value &,const vector<Json::Value> &)> FetchSearchCallback;
			function<void(const string &,const Json::Value &,const FetchSearchCallback &)> fetchSearch;
			function<void(const string &,const function<void(const vector<Json::Value> &)>)> fetchDynamic;
			function<void(const string &,const std::map<string,string> &)> removePlaylist;

			void validate() const;
		};

		DbSession(const shared_ptr<IApp> &app, const Behaviour &behaviour);
    
    	std::string sessionIdentifier() const;
    	void removeData();
    	void connect(const string &user, const string &pass, const function<void(bool)> &callback);
    	virtual void refresh() override final;
		const Behaviour behaviour;

        virtual void removePlaylist(const shared_ptr<IPlaylist> &playlist);

        virtual void playbackData(const ISong &song, const function<void(const shared_ptr<IPlaybackData> &)> data) const override final;
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<ISong> &song) const override final;
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<IPlaylist> &song) const override final;

        virtual vector<shared_ptr<IPlaylist>> playlists() override;

        virtual void search(const std::string &filter);

        virtual void fetchDynamic(const string &playlistId) override;
        virtual void clearDynamic(const string &playlistId) override;
        virtual void createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result) override;

	private:
		void gotData(const string &entity,const vector<Json::Value> &items, bool finishedWithEntity);
	    void fetchLibrary();

	    string _currentFilter;
		std::map<string,vector<string>> _toRemove;
		std::map<string,bool> _toRemoveStartedCounting;
		size_t _numberOfSongsBeforeFetch;
	};
}

#endif
