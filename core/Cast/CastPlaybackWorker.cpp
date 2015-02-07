//
//  CastPlaybackWorker.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 14/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include "CastPlaybackWorker.h"
#include "ChromeCast.h"
#include "CastDevice.h"
#include "BackgroundExecutor.h"
#include "CastApplication.h"
#include "json.h"
#include "CastSession.h"
#include "Streamer.h"
#include "IAudioGenerator.h"
#include "Timer.h"
#include "IApp.h"
#include "IPlayer.h"

namespace Gear
{
    using namespace Cast;
    using namespace Gear;
    
#define method CastPlaybackWorker::
    
    shared_ptr<CastPlaybackWorker> method create()
    {
        shared_ptr<CastPlaybackWorker> ret(new CastPlaybackWorker());
        ret->init();
        return ret;
    }
    
    method CastPlaybackWorker() :
        _probeTimer(ChromeCast::instance()->io())
    {
        _playing = false;
        _volume = 0;
    }
    
    void method probe()
    {
        auto instance = ChromeCast::instance();
        instance->probeAsync();
        _probeTimer.expires_from_now(boost::posix_time::seconds(40));
        
        auto self = shared_from_this();
        _probeTimer.async_wait([self](const boost::system::error_code e){
            bool change = false;
            self->_devices.set([&](vector<AudioDevice> &v){
                v.erase(remove_if(v.begin(), v.end(), [&](const AudioDevice &device){
                    if (device.lastSeenSecondsBefore() > 170) {
                        std::cout << "CLOSE removing cc because we have not seen it for a while\n";
                        change = true;
                        return true;
                    } else {
                        return false;
                    }
                }), v.end());
            });

            if (change) {
                Gear::IApp::instance()->playbackWorker()->reload();
            }

            self->probe();
        });
    }
    
    static uint32_t convertAddress(const string &ip)
    {
        std::stringstream s(ip);
        int a,b,c,d; //to store the 4 ints
        char ch; //to temporarily store the '.'
        s >> a >> ch >> b >> ch >> c >> ch >> d;
        uint32_t ret = (a << 24) | (b << 16) | (c << 8) | d;
        return ret;
    }
    
    static std::string convertAddress(uint32_t ip)
    {
        std::stringstream s;
        s << ((ip >> 24)&0xff) << "." << ((ip >> 16)&0xff) << "." << ((ip >>8)&0xff) << "." << (ip & 0xff);
        return s.str();
    }
    
    AudioDevice method convertDevice(const CastDevice &device)
    {
        return AudioDevice(convertAddress(device.address()), device.name(), shared_from_this());
    }
    
    void method foundDevice(const shared_ptr<CastDevice> &device)
    {
        auto audioDevice = convertDevice(*device);
        _devices.set([&](vector<AudioDevice> &v){
            auto it = find(v.begin(), v.end(), audioDevice);
            if (it == v.end()) {
                v.push_back(audioDevice);
            } else {
                it->updateLastSeen();
            }
        });
    }

    void method deviceDown(const string &ip)
    {
        std::cout << "device down: " << ip << std::endl;
        AudioDevice device(convertAddress(ip), "", shared_from_this());
        _devices.set([&](vector<AudioDevice> &v){

            v.erase(remove(v.begin(), v.end(), device), v.end());
        });

        Gear::IApp::instance()->playbackWorker()->reload();
    }
    
    void method init()
    {
        auto self = shared_from_this();
        
        Base::BackgroundExecutor::instance().addTask([self]{
            auto instance = ChromeCast::instance();
            instance->setFoundCallback([self](const shared_ptr<CastDevice> &device){
                self->foundDevice(device);
            });
            
            instance->scanAsync();
            self->probe();
            
            instance->runSync();
        });
    }
    
    void method play()
    {
        _playing = true;
    }
    
    void method stop(bool release)
    {
        _playing = false;
    }
    
    void method setVolume(float volume)
    {
        _volume = volume;
        auto self = shared_from_this();
        ChromeCast::instance()->io().post([self, volume]{
            Json::Value msg(Json::objectValue);
            msg["volume"] = volume;

            for (auto &device : self->_activeDevices) {
                auto application = device->application();
                if (application) {
                    application->send(msg);
                }
            }
        });
    }
    
    vector<AudioDevice> method availableDevices() const
    {
        return (vector<AudioDevice>)_devices;
    }
    
    void method setDevices(const vector<AudioDevice> &devices)
    {
        IPlaybackWorker::setDevices(devices);

        auto self = shared_from_this();
        ChromeCast::instance()->io().post([self, devices]{

            self->_activeDevices.erase(remove_if(self->_activeDevices.begin(),self->_activeDevices.end(), [&](const shared_ptr<CastDevice> &active){
                if (!active->connected()) {
                    return true;
                }
                return false;
            }), self->_activeDevices.end());

            self->_activeDevices.erase(remove_if(self->_activeDevices.begin(),self->_activeDevices.end(), [&](const shared_ptr<CastDevice> &active){
                
                bool found = false;
                for (const AudioDevice &device : devices) {
                    string host = convertAddress(device.deviceId());
                    if (active->address() == host) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    active->stopApplication();
                    return true;
                }
                return false;
            }), self->_activeDevices.end());

            for (const AudioDevice &device : devices) {
                string host = convertAddress(device.deviceId());
                bool found = false;
                for (const shared_ptr<CastDevice> &active : self->_activeDevices) {
                    if (active->address() == host) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    continue;
                }
                
                auto toConnect = CastDevice::create(device.name(), host);
                self->_activeDevices.push_back(toConnect);
                self->connect(toConnect);
            }
        });
    }

    void method terminate()
    {
        auto self = shared_from_this();
        ChromeCast::instance()->io().post([self]{
            for (auto &d : self->_activeDevices) {
                d->stopApplication();
            }
        });
    }

/*
    static int64_t milliseconds()
    {
        struct timeval val;
        gettimeofday(&val, NULL);
        return val.tv_sec * 1000 + val.tv_usec / 1000;
    }
*/
    
    void method connect(const shared_ptr<CastDevice> &device)
    {
        //std::cout << "connecting\n";
        
        if (!_streamer) {
            _streamer = shared_ptr<Streamer>(new Streamer(ChromeCast::instance()->io(), shared_from_this()));
            
            auto streamer = _streamer;
            ATOMIC_NS::atomic_bool &playing = _playing;
            auto generator = _generator;
            auto &io = ChromeCast::instance()->io();
            static auto begin = CHRONO_NS::steady_clock::now();
            static long lag = 0;
            static uint64_t samplesPerChannelSoFar = 0;
            static bool previouslyPlaying = false;

            static SignalConnection pauseConnection = IApp::instance()->player()->playingConnector().connect([streamer,&io](bool playing){
                if (!playing) {
                    io.post([streamer]{
                        streamer->cancelStreaming();
                    });
                }
            });

            new Base::Timer([generator,streamer,&playing,&io](Timer *timer){

                bool isPlaying = playing;
                if (isPlaying) {
                    if (!previouslyPlaying) {
                        previouslyPlaying = true;
                        IApp::instance()->disableAppNap();

                        /*begin = steady_clock::now();
                        lag = 0;
                        samplesPerChannelSoFar = 0;*/
                    }
                } else {
                    if (previouslyPlaying) {
                        previouslyPlaying = false;
                        IApp::instance()->enableAppNap();

                        /*io.post([streamer]{
                            // empty buffers
                            streamer->clearBuffer();
                        });*/
                    }
                    // we shouldn't return as we are stil counting... and most importantly, streaming silence!
                    //return;
                }

                static const int kBufferSize = 4410;
                
                auto now = CHRONO_NS::steady_clock::now();
                auto elapsed = CHRONO_NS::duration_cast<CHRONO_NS::milliseconds>(now-begin).count();
                //std::cout << "timer called... " << elapsed << "\n";
                
                do {
                    //std::cout << "timer entry...\n";
                    
                    //auto elapsed = now - begin;

                    // a bit early to buffer
                    // prebuffering is wrong here! it just offsets problems in time
                    auto nextMillisec = samplesPerChannelSoFar * 1000 / 44100;
                    int64_t w = 0;
                    if (lag == 0) {
                        w = nextMillisec - elapsed;
                    }

                    if (w > 100) {
                        return;
                    }


                    #if 1
                    io.post([generator,streamer,isPlaying]{
                        float buf[kBufferSize];
                        size_t gotFloatAll = 0;

                        // we shouldn't disturb getUncompressedDataInto if we just want to stream silence
                        if (isPlaying) {
                            while(true) {
                                auto got = generator->getUncompressedDataInto(reinterpret_cast<char *>(buf + gotFloatAll), (kBufferSize - gotFloatAll) * sizeof(float), 0);
                                if (got == 0) {
                                    break;
                                }

                                auto gotFloat = got/sizeof(float);
                                gotFloatAll += gotFloat;
                                if (gotFloatAll >= kBufferSize) {
                                    break;
                                }
                            }
                        } else {
                            generator->silence(0);
                        }
                        
                        if (gotFloatAll == 0) {
                            streamer->streamSilence(); 
                            //std::cout << "sending silence 4410 floats\n"; 
            
                            return;
                        }
                        
                        // this would insert emptiness unnecessarily
                        //if (gotFloatAll < kBufferSize) {
                        //if (gotFloatAll == 0 && !playing) {
                        //    memset(buf + gotFloatAll, 0, (kBufferSize-gotFloatAll) * sizeof(float));
                        //}

                        //std::cout << "sending music " << gotFloatAll << " floats\n"; 

                        streamer->writeSamplesFromIo(buf, gotFloatAll);
                        
                        /*FILE *f = fopen("/Users/zsszatmari/tmp/test2.raw", "a");
                         fwrite(buf, 1, kBufferSize * sizeof(float), f);
                         fclose(f);*/
                    });
                    #endif
                    
                    
                    samplesPerChannelSoFar += kBufferSize/2;

                    
                    if (lag > 0) {
                        //std::cout << "large wait: " << w << " samples so far: " << samplesPerChannelSoFar << " nextmillis: " << nextMillisec << " lag: " << lag << " elapsed: " << elapsed << std::endl;
                    }
                    
                    static int64_t largestWait = 0;
                    if (w > 0) {
                        lag = 0;
                    } else {
                        lag = -w;
                        //std::cout << "lagging behind... " << lag << " largest wait so far: " << largestWait << " nextmilli: " << nextMillisec << " elapsed: " << elapsed << std::endl;
                    }
                } while (lag > 0);

                elapsed = CHRONO_NS::duration_cast<CHRONO_NS::milliseconds>(now-begin).count();
            }, 0.050f, true, 0.040f);
        }
        
        auto port = _streamer->port();
        auto instance = ChromeCast::instance();
        float volume = _volume;
        device->connect(instance->io(), instance->sslContext(), [port,device,volume](int err){
            
            if (err) {
                return;
            }
            
            device->application("E1552DB1",[port,device,volume](int err, const shared_ptr<CastApplication> &application){
                device->setApplication(application);

                std::stringstream ss;
                ss << "http://" << device->channel()->localHost() << ":" << port;
                string localHostUrl = ss.str();
                application->setLocalHostUrl(localHostUrl);
                
                application->run("urn:x-cast:com.treasurebox.gear", [port,device,localHostUrl,volume](int err, const shared_ptr<CastSession> &session){
                    Json::Value data = Json::objectValue;
                    //data["media"] = "http://" + device->channel()->localHost() + "/07Venus.mp3";
                    //data["media"] = "http://" + device->channel()->localHost() + "/long.ogg";
                    
                    data["volume"] = volume;
                    data["media"] = localHostUrl;
                    data["playing"] = IApp::instance()->player()->playingConnector().value();
                    
                    session->send(data, [](int ec, const Json::Value &reply){
                        return true;
                    });
                });
            });
        });
    }
    
    int method affinity() const
    {
        return -1;
    }
}