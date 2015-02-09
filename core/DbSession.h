#ifndef DBSESSION_H
#define DBSESSION_H

#include <memory>
#include "SongManipulationSession.h"

namespace Gear
{
	using std::string;
	using std::shared_ptr;

	class DbSession : public SongManipulationSession
	{
	public:
		struct Behaviour
		{
			string sourceIdentifier;

			void validate() const;
		};

		DbSession(const shared_ptr<IApp> &app, const Behaviour &behaviour);
    
    	std::string sessionIdentifier() const;
    	void removeData();

	private:
		Behaviour _behaviour;
	};
}

#endif
