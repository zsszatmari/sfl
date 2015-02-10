//
//  CreateRadioIntent.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/5/13.
//
//

#include "CreateRadioIntent.h"
#include "BackgroundExecutor.h"

namespace Gear
{
#define method CreateRadioIntent::
    
    method CreateRadioIntent(const shared_ptr<GooglePlaySession> &session, const shared_ptr<IApp> &app, const string &fieldHint) :
        _session(session),
        _app(app),
        _fieldHint(fieldHint)
    {
    }
    
    const string method menuText() const
    {
        string ret;
        if (_session->allAccessCapable()) {
            ret = u("Start Radio");
        } else {
            ret = u("Create Instant Mix");
        }
        
        if (_fieldHint == "artist") {
            ret.append(u(" for Artist"));
        } else if (_fieldHint == "album") {
            ret.append(u(" for Album"));
        }
        
        return ret;
    }
    
    void method apply(const vector<shared_ptr<ISong>> &songs) const
    {
        if (songs.size() == 1) {
            const shared_ptr<ISong> song = songs.at(0);
            auto localSession = _session;
            auto localApp =_app;
            auto hint = _fieldHint;
            
            Base::BackgroundExecutor::instance().addTask([song,localApp,localSession, hint]{

                shared_ptr<ISong> localSong = song;
                auto createdRadio = localSession->createRadioSync(localSong, hint);
                if (createdRadio) {
                	localApp->userSelectedCategory(kRadioTag, createdRadio, true);
                }
            });
        }
    }
}
