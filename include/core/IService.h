//
//  IService.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#ifndef __G_Ear_Player__IService__
#define __G_Ear_Player__IService__

#include <string>
#include "ManagedValue.h"
#include "stdplus.h"

namespace Gear
{
    using std::string;
    using namespace Base;
    
    class IService
    {
    public:
        enum class State
        {
            Offline,
            Connecting,
            Online
        };
        
        IService();
        virtual ~IService();
        virtual string title() const = 0;
// some obfuscation here...
#ifndef inAppIdentifier
#define inAppIdentifier songs
#endif
        virtual string inAppIdentifier() const = 0;
#ifndef inAppNecessary
#define inAppNecessary art
        virtual bool inAppNecessary() const = 0;
#endif
        
        // connect only if credentials are saved
        virtual void autoconnect() = 0;
        // try to connect, possibly asking for credentials
        virtual void connect() = 0;
        virtual void disconnect() = 0;
        virtual bool disabled() const;
        ValueConnector<State> stateConnector();
        State state() const;
        
    protected:
        ManagedValue<State> _state;
    };
}

#endif /* defined(__G_Ear_Player__IService__) */
