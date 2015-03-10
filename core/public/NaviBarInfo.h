#ifndef NAVIBAR_H
#define NAVIBAR_H

#include <vector>
#include <string>
#include "stdplus.h"
#include MEMORY_H

namespace Gear
{
	using std::string;
	using std::vector;
	using std::pair;
	using MEMORY_NS::shared_ptr;

	class IPlaylist;

	class NaviBarInfo final
	{
	public:
		NaviBarInfo();
		NaviBarInfo(const vector<pair<string,shared_ptr<IPlaylist>>> &stack, const vector<pair<string,shared_ptr<IPlaylist>>> &views);

		static NaviBarInfo forPlaylist(const shared_ptr<IPlaylist> &playlist);

		vector<pair<string,shared_ptr<IPlaylist>>> stack;
		vector<pair<string,shared_ptr<IPlaylist>>> views;
	};
}

#endif