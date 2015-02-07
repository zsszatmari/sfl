//
//  CastApplication.h
//  Cast
//
//  Created by Zsolt Szatmari on 01/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__CastApplication__
#define __Cast__CastApplication__

#include <functional>
#include "stdplus.h"
#include "SignalConnection.h"
#include SHAREDFROMTHIS_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Cast
{
    class CastSession;
    class CastChannel;
    class CastDevice;
    
    using std::string;
    using std::function;
    
    class CastApplication final : public MEMORY_NS::enable_shared_from_this<CastApplication>
    {
    public:
        static shared_ptr<CastApplication> create(const string &appId, const shared_ptr<CastChannel> &channel, const shared_ptr<CastDevice> &device);
        
        void run(const std::string &namespace_id, const function<void(int error, const shared_ptr<CastSession> &session)> &handler);
        virtual ~CastApplication();
        void setLocalHostUrl(const std::string &localHostUrl);
        void send(const Json::Value &data);

    private:
        CastApplication(const string &appId, const shared_ptr<CastChannel> &channel, const shared_ptr<CastDevice> &device);
        void displayData(const shared_ptr<CastSession> &session);
        void addAlbumArt(Json::Value &value);

        string _appId;
        string _localHostUrl;
        shared_ptr<CastChannel> _channel;
        shared_ptr<CastDevice> _device;
        shared_ptr<CastSession> _session;
        float _previousRatio;

        Base::SignalConnection _songConnection;
        Base::SignalConnection _ratioConnection;
        Base::SignalConnection _artConnection;
        Base::SignalConnection _queueConnection;
        Base::SignalConnection _playingConnection;
    };
}

#endif /* defined(__Cast__CastApplication__) */
