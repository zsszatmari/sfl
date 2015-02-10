#ifndef PLAYERSWITCH_H
#define PLAYERSWITCH_H

#include "IPlayer.h"
#include "stdplus.h"

namespace Gear
{
	class PlayerSwitch final : public IPlayer
	{
	public:
		PlayerSwitch(const shared_ptr<IPlayer> _player);
		void push(const shared_ptr<IPlayer> _player);
		void pop();

		virtual void play(const SongEntry &song);
        virtual void prev();
        virtual void play();
        virtual void next();
        virtual void rate(const shared_ptr<ISong> &song);
        virtual void rate(const shared_ptr<ISong> &song, int selected);
        virtual void rate();
        virtual void setRatio(float ratio);

        shared_ptr<PromisedImage> albumArt(const long imageSize) const;
        SongGrouping groupingCurrentlyPlaying() const;

	private:
		shared_ptr<IPlayer> current() const;

		void refreshConnections();

		std::vector<shared_ptr<IPlayer>> _players;
		std::vector<SignalConnection> _conns;
	};
}


#endif