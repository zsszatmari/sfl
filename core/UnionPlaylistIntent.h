#ifndef UNIONPLAYLISTINTENT_H
#define UNIONPLAYLISTINTENT_H

#include <vector>
#include "IPlaylistIntent.h"
#include "stdplus.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
	class UnionPlaylistIntent : public IPlaylistIntent
	{
	public:
		UnionPlaylistIntent(const std::string &s, const std::vector<shared_ptr<IPlaylistIntent>> &elements);

		virtual const std::string menuText() const;
        virtual bool confirmationNeeded();
        virtual const std::string confirmationText() const;
        virtual void apply();

    private:
    	const std::string _menuText;
    	const std::vector<shared_ptr<IPlaylistIntent>> _elements;
	};
}

#endif