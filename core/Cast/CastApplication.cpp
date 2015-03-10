//
//  CastApplication.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 01/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include "json.h"
#include "CastApplication.h"
#include "CastChannel.h"
#include "CastSession.h"
#include "IApp.h"
#include "IPlayer.h"
#include "SongEntry.h"
#include "QueueSongArray.h"
#include "PromisedImage.h"
#include "NamedImage.h"
#include "BitmapImage.h"
#include "Logger.h"

namespace Cast
{
#define method CastApplication::
 
    using namespace Gear;
    using namespace Gui;

    shared_ptr<CastApplication> method create(const string &appId, const shared_ptr<CastChannel> &channel, const shared_ptr<CastDevice> &device)
    {
        return shared_ptr<CastApplication>(new CastApplication(appId, channel, device));
    }
    
    method CastApplication(const string &appId, const shared_ptr<CastChannel> &channel, const shared_ptr<CastDevice> &device) :
        _appId(appId),
        _channel(channel),
        _device(device),
        _previousRatio(0)
    {
    }

    method ~CastApplication()
    {
        Json::Value json(Json::objectValue);
        json["shutdown"] = true;

        if (_session) {
            _session->send(json, [](int,const Json::Value &){
                return true;
            });
        }
    }
    
    void method run(const std::string &namespace_id, const function<void(int error, const shared_ptr<CastSession> &session)> &handler)
    {
        Json::Value msg;
        msg["namespace"] = "urn:x-cast:com.google.cast.receiver";
        Json::Value data;
        data["type"] = "LAUNCH";
        data["appId"] = _appId;
        msg["data"] = data;
        auto self = shared_from_this();
        _channel->send(msg, [self,handler,namespace_id](int err, const Json::Value &reply){
            if (err) {
                handler(err, nullptr);
                return true;
            }
            if (reply.get("data", Json::objectValue).get("type","") != "RECEIVER_STATUS") {
                // we are not there yet
                //handler(2, nullptr);
                return false;
            }
            Json::Value status = reply.get("data", Json::objectValue).get("status",Json::objectValue);
            Json::Value apps = status.get("applications", Json::arrayValue);
            bool found = false;
            Json::Value app;
            for (const auto &appIter : apps) {
                if (appIter.get("appId","").asString() == self->_appId) {
                    app = appIter;
                    found = true;
                    break;
                }
            }
            if (!found) {
                Json::FastWriter writer;
                string data = writer.write(reply);
                
                std::cout << "app not running\n";
                //handler(11 ,nullptr);
                return false;
            }
            if (!namespace_id.empty()) {
                auto namespaces = app.get("namespaces", Json::arrayValue);
                found = false;
                for (const auto &nsIter : namespaces) {
                    if (nsIter.get("name","").asString() == namespace_id) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    std::cout << "requested namespace not found\n";
                    handler(12, nullptr);
                    return true;
                }
            }
            
            std::cout << "launched app\n";
            auto session = CastSession::create(self->_device, self, namespace_id, app.get("transportId","").asString());
            self->displayData(session);
            handler(0, session);

            return true;
        });
    }

    void method setLocalHostUrl(const std::string &url)
    {
        _localHostUrl = url;
    }

    static Json::Value songToJson(const SongEntry &song)
    {
        auto s = song.song();
        Json::Value songJson(Json::objectValue);
        if (s) {
            songJson["artist"] = s->artist();
            songJson["album"] = s->album();
            songJson["title"] = (std::string)s->title();
            songJson["durationMillis"] = s->durationMillis();
            songJson["entryId"] = song.entryId();
            songJson["uniqueId"] = s->uniqueId();
        } else {
            songJson["artist"] = "";
            songJson["album"] = "";
            songJson["title"] = "";
            songJson["durationMillis"] = 0;
            songJson["entryId"] = "";
            songJson["uniqueId"] = "";
        }
        return songJson;
    }

    void method addAlbumArt(Json::Value &value)
    {
        static int i = 0;
        std::stringstream ss;
        ss << _localHostUrl << "/" << ++i << ".png";
#ifdef DEBUG
        Logger::stream() << "album art callback: " << ss.str();
#endif

        value["albumArtUrl"] = ss.str();
    }

    void method send(const Json::Value &data)
    {
        if (_session) {
            _session->send(data, [](int, const Json::Value &){
                return true;
            });
        }
    }

    void method displayData(const shared_ptr<CastSession> &session)
    {
        // retain session
        _session = session;

        auto player = IApp::instance()->player();
        weak_ptr<CastSession> wsession = session;
        weak_ptr<CastApplication> wself = shared_from_this();

        _songConnection = player->songEntryConnector().connect([wsession,wself](const SongEntry &song){
            auto session = wsession.lock();
            if (session) {
                Json::Value value(Json::objectValue);

                value["currentSong"] = songToJson(song);;
            
                /*auto self = wself.lock();
                if (self) {
                    self->addAlbumArt(value);
                }*/
                session->send(value, [](int,const Json::Value &){
                    return true;
                });

                auto promise = IApp::instance()->player()->albumArt(512);
                //shared_ptr<bool> first(new bool(true));
                auto self = wself.lock();
                if (!self) {
                    return;
                }
                self->_artConnection = promise->connector().connect([wself,wsession,song](const shared_ptr<IPaintable> &paintable){
                    
                    // we have to send the new album art even in the case that we 
                    // have no first, but almost immediately we have one!
                    /*if (*first) {
                        *first = false;
                        return;
                    }*/
                    // this is for debugging:
                    /*auto named = dynamic_pointer_cast<NamedImage>(paintable);
                    if (named) {
                        std::cout << "album art updated for " << (std::string)song.fastTitle() << " to named " << named->imageName() << std::endl;
                    }
                    auto bitmap = dynamic_pointer_cast<BitmapImage>(paintable);
                    if (bitmap) {
                        std::cout << "album art updated for " << (std::string)song.fastTitle() << " to bitmap" << std::endl;
                    }*/

                    auto self = wself.lock();
                    if (self) {
                        Json::Value value(Json::objectValue);
                        self->addAlbumArt(value);
                        auto session = wsession.lock();
                        if (session) {
                            session->send(value, [](int,const Json::Value &){
                                return true;
                            });
                        }
                    }
                });
            }
        });

        _ratioConnection = player->songRatioConnector().connect([wself,wsession](float value){
            auto session = wsession.lock();
            auto self = wself.lock();
            if (session && self && (value < self->_previousRatio || value >= self->_previousRatio + 0.005f)) {

                self->_previousRatio = value;

                Json::Value json(Json::objectValue);
                json["songRatio"] = value;
                
                session->send(json, [](int,const Json::Value &){
                    return true;
                });
            }
        });
        _playingConnection = player->playingConnector().connect([wsession](bool playing){
            auto session = wsession.lock();

            if (session) {
                Json::Value json(Json::objectValue);
                json["playing"] = playing;

                session->send(json, [](int,const Json::Value &){
                    return true;
                });
            }
        });

        auto c = [wsession]{

            auto session = wsession.lock();
            if (session) {
                Json::Value json(Json::objectValue);
                Json::Value elements(Json::arrayValue);

                #pragma message("TODO: ChromeCast song display")
                /*auto songs = QueueSongArray::instance()->songs();
                for (int i = 0 ; i < songs->size() ; ++i) {
                    elements.append(songToJson(songs->at(i)));
                }*/

                json["queue"] = elements;
                session->send(json, [](int,const Json::Value &){
                    return true;
                });
            }
        };
        _queueConnection = QueueSongArray::instance()->updatedEvent().connect(c);
        c();
    }

}