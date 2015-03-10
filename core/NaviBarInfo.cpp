#include <string>
#include "NaviBarInfo.h"
#include "NaviBarFn.h"

namespace Gear
{
#define method NaviBarInfo::

	method NaviBarInfo()
	{
	}

	method NaviBarInfo(const vector<pair<string,shared_ptr<IPlaylist>>> &aStack, const vector<pair<string,shared_ptr<IPlaylist>>> &aViews) :
		stack(aStack),
		views(aViews)
	{
	}

	NaviBarInfo method forPlaylist(const shared_ptr<IPlaylist> &playlist)
	{
		return NaviBarFn::info(playlist);
	}
}