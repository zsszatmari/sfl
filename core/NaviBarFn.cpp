#include "NaviBarFn.h"
#include "sfl/Prelude.h"
#include "GroupedPlaylist.h"
#include "SortedSongArray.h"

using std::make_pair;
using namespace sfl;

namespace Gear
{
	namespace NaviBarFn
	{
		static vector<pair<string,vector<string>>> standardGroupings()
		{
		    return vector<std::pair<string,vector<string>>>({
		    	std::make_pair("Songs",vector<string>{}),
				std::make_pair("Artists",vector<string>{"artist"}),
				std::make_pair("Albums",vector<string>{"album","artist"}),
				std::make_pair("Genres",vector<string>{"genre"}),
				std::make_pair("Album Artists",vector<string>{"albumArtist"})
		                                                });
		}

		static NaviBarInfo rootAll(const shared_ptr<IPlaylist> &playlist)
		{
			vector<pair<string,shared_ptr<IPlaylist>>> stack;
			auto views = map([&](const std::pair<string,vector<string>> &p){
				shared_ptr<IPlaylist> grouped = p.second.empty() 
								? playlist 
								: GroupedPlaylist::create(MEMORY_NS::dynamic_pointer_cast<SortedSongArray>(playlist->songArray()), p.first, p.second, playlist);
				return make_pair(p.first, grouped);
			}, standardGroupings());
			return NaviBarInfo(stack, views);
		}

		static NaviBarInfo addLevel(NaviBarInfo info, const shared_ptr<IPlaylist> &playlist)
		{
			info.stack.push_back(make_pair(playlist->name(),playlist));
			return info;
		}

		NaviBarInfo info(const shared_ptr<IPlaylist> &playlist)
		{
			auto pid = playlist->playlistId();
			auto prev = playlist->previous();
			return prev 
					? addLevel(info(prev), playlist)
					: (pid == "all" || pid == "free") 
						? rootAll(playlist)
						: NaviBarInfo();
		}
	}
}