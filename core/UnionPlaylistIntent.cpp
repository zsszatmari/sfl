#include <sstream>
#include "UnionPlaylistIntent.h"
#include MEMORY_H

namespace Gear
{
#define method UnionPlaylistIntent::

	method UnionPlaylistIntent(const std::string &s, const std::vector<shared_ptr<IPlaylistIntent>> &elements) :
		_menuText(s),
		_elements(elements)
	{
	}

	const std::string method menuText() const
	{
		return _menuText;
	}

    bool method confirmationNeeded()
    {
    	return true;
    }

    const std::string method confirmationText() const
    {
    	if (_menuText == "Delete") {
    		std::stringstream ss;
    		ss << "Are you sure you wish to delete these " << _elements.size() << " playlists?";
    		return ss.str();
    	} else {
    		return "Are you sure?";
    	}
    }

    void method apply()
    {
    	for (auto &element : _elements) {
    		element->apply();
    	}
    }
}
