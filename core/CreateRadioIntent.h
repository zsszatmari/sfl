//
//  CreateRadioIntent.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/5/13.
//
//

#ifndef __G_Ear__CreateRadioIntent__
#define __G_Ear__CreateRadioIntent__

#include "ISongNoIndexIntent.h"
#include "ISession.h"
#include "GooglePlaySession.h"
#include "IApp.h"

namespace Gear
{
    class CreateRadioIntent : public ISongNoIndexIntent
    {
    public:
        CreateRadioIntent(const shared_ptr<GooglePlaySession> &session, const shared_ptr<IApp> &app, const string &fieldHint);
        virtual const string menuText() const;
        virtual void apply(const vector<shared_ptr<ISong>> &songs) const;
        
    private:
        const shared_ptr<GooglePlaySession> _session;
        const shared_ptr<IApp> _app;
        const string _fieldHint;
    };
}

#endif /* defined(__G_Ear__CreateRadioIntent__) */
