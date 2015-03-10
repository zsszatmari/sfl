#ifndef PLAYER_REMOTE_H
#define PLAYER_REMOTE_H

#include "IPlayer.h"
#include "stdplus.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
	class RemoteControl;

	class PlayerRemote final : public IPlayer, public MEMORY_NS::enable_shared_from_this<PlayerRemote>
	{
	public:
		static shared_ptr<PlayerRemote> create(const shared_ptr<RemoteControl> &control);

		virtual void play(const SongEntry &song);
        virtual void prev();
        virtual void play();
        virtual void next();
        virtual void rate(const shared_ptr<ISong> &song);
        virtual void rate(const shared_ptr<ISong> &song, int selected);
        virtual void rate();
        virtual void setRatio(float ratio);

	private:
		PlayerRemote(const shared_ptr<RemoteControl> &control);
		void initRemote();

        virtual void play(const shared_ptr<IPlaylist> &playlist, const SongEntry &song, bool forceRestart = false);

		weak_ptr<RemoteControl> _control;
	};
}

#endif