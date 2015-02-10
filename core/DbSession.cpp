#include "DbSession.h"
#include "IApp.h"
#include "Db.h"

namespace Gear
{
#define method DbSession::

	method DbSession(const shared_ptr<IApp> &app, const Behaviour &behaviour) :
		SongManipulationSession(app),
		_behaviour(behaviour)
	{
		_behaviour.validate();
	}

	void DbSession::Behaviour::validate() const
	{
		assert(!sourceIdentifier.empty());
	}

    std::string method sessionIdentifier() const
    {
        return _behaviour.sourceIdentifier;
    }

   	void method removeData()
   	{
   		IApp::instance()->db()->remove("Song", ClientDb::Predicate("source", _behaviour.sourceIdentifier));
   		IApp::instance()->db()->remove("Playlist", ClientDb::Predicate("source", _behaviour.sourceIdentifier));
   	}
}
