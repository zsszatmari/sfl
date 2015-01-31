//
//  LastFmController.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/17/12.
//
//

#include "ISong.h"
#include "stdplus.h"
#include "LastFmProtocol.h"
#include "SerialExecutor.h"
#include "ManagedValue.h"
#include "SerialExecutor.h"
#include CHRONO_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
    using std::string;
    
    class LastFmController final : public MEMORY_NS::enable_shared_from_this<LastFmController>
    {
    public:
        static shared_ptr<LastFmController> create();
        void applicationDidBecomeActive();
        void songBegan(const shared_ptr<ISong> &aSong);
        void playTimePassed(float time);
        void setEnabled(bool enabled);
        void loginMobile(const string &user, const string &pass);

        Base::ValueConnector<bool> enabledConnector();
        Base::ValueConnector<std::string> messageConnector();
        
    private:
        LastFmController();
        
        string sessionKey();
        void forget();
        void saveSessionKey(const string &sessionKey, const string &name);
        bool enabled();
        void authenticate();
        void setLoggedInMessage();
        void scrobbleIfNeeded();
        
        Base::ManagedValue<bool> _enabled;
        Base::ManagedValue<std::string> _message;
        string _scrobbledMessage;
        
        shared_ptr<LastFmProtocol> _protocol;
        string _name;
        Base::SerialExecutor _queue;
        float _timeLeftToScrobble;
        
        shared_ptr<ISong> _song;
        CHRONO_NS::system_clock::time_point _songStarted;
        bool _scrobbled;
    };
}
