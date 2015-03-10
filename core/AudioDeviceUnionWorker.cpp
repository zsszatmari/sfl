//
//  AudioDeviceUnionEnumerator.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 13/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include "AudioDeviceUnionWorker.h"
#include "IApp.h"
#include "IPreferences.h"
#include "json.h"
#include "BackgroundExecutor.h"
#include "PlaybackController.h"
#include "IEqualizer.h"
#include "PlaybackItem.h"
#include <thread>
#include "Logger.h"

namespace Gear
{

#if TARGET_OS_IPHONE
    // lock screen needs much larger buffer due to energy efficiency reasons
#define UNION_SINGLEOUTPUT
#endif

#define method AudioDeviceUnionWorker::
    
    static vector<shared_ptr<IPlaybackWorker>> two(const shared_ptr<IPlaybackWorker> &first, const shared_ptr<IPlaybackWorker> &second)
    {
        vector<shared_ptr<IPlaybackWorker>> ret;
        ret.push_back(first);
        ret.push_back(second);
        return ret;
    }
    
    shared_ptr<AudioDeviceUnionWorker> method create(const shared_ptr<IPlaybackWorker> &first, const shared_ptr<IPlaybackWorker> &second)
    {
        return create(two(first,second));
    }
    
    shared_ptr<AudioDeviceUnionWorker> method create(const vector<shared_ptr<IPlaybackWorker>> workers)
    {
        shared_ptr<AudioDeviceUnionWorker> ret(new AudioDeviceUnionWorker(workers));
        ret->init();
        return ret;
    }
    
    method AudioDeviceUnionWorker(const vector<shared_ptr<IPlaybackWorker>> workers) :
        _workers(workers),
        _equalizer(IApp::instance()->equalizer())
    {
        _playing = false;
    }

    // the old 'SelectedOutputDevice' is now deprecated
    static const std::string kKeyDevices = "SelectedOutputDevices";
    
    static uint32_t doFill(CircularBuffer &buf, char *dataPtr, int gotBytes)
    {
        int32_t available;
        void *head = buf.head(available);
        if (head) {
            if (available > gotBytes) {
                available = gotBytes;
            } 
            memcpy(head, dataPtr, available);
            buf.produce(available);
            return available;
        } else {
#ifdef DEBUG
            //std::cout << "hm1\n";
#endif
        }
        return 0;
    }
    
    static void fill(CircularBuffer &buf, char *dataPtr, int gotBytes)
    {       
        auto filled = doFill(buf, dataPtr, gotBytes);
        if (filled > 0 && filled < gotBytes) {
            doFill(buf, dataPtr + filled, gotBytes - filled);
        }
    }

    void method initEq(size_t bands)
    {
    }

    void method processEq(char *data, size_t num)
    {
        if (_equalizer) {
            _equalizer->process(data, num);
        }
    }

    void method init()
    {
#ifdef ENABLE_UNIVERSAL_EQ
        initEq(10);
#endif

        for (auto &w : _workers) {
            w->setGenerator(shared_from_this());
        }

        reload();

        Base::BackgroundExecutor::instance().addTask([this]{
#ifndef UNION_SINGLEOUTPUT
            while(true) {
                while (_playing) {

                    bool notEnough = false;
                    for (int i = 0 ; i < kBufferNum ; ++i) {
                        
                        int32_t available;
                        if (!_active[i]) {
                            continue;
                        }
                        _buffers[i].tailTotal(available);
                        if (available < 22100) {
                            notEnough = true;
                            break;
                        }
                    }
                    if (notEnough) {

                        char data[1024];
                        auto gotBytes = _generator->getUncompressedDataInto(data, sizeof(data), 0);
                        debugStreamAppend("decoded", data, gotBytes);

                        processEq(data, gotBytes);

                        for (int i = 0 ; i < kBufferNum ; ++i) {
                            if (_active[i]) {
                                fill(_buffers[i], data, gotBytes);
                            }
                        }
                    } else {
                        break;
                    }
                }

                CHRONO_NS::milliseconds w(30);
                THREAD_NS::this_thread::sleep_for(w);
            }
#endif
        });
    }
    
    void method reload()
    {
        Json::Reader parser;
        Json::Value value;
        std::string str = IApp::instance()->preferences().stringForKey(kKeyDevices);
#ifdef DEBUG
        std::cout << "hm: " << str << std::endl;
#endif
        parser.parse(str, value);
        vector<AudioDevice> success;
        if (value.isArray()) {
            for (int workerId = 0 ; workerId < value.size() ; ++workerId) {

                if (workerId >= _workers.size()) {
                    break;
                }

                vector<long long> deviceIds;
                Json::Value nums = value[workerId];
                for (int i = 0 ; i < nums.size() ; ++i) {
                    auto deviceId = nums[i].asInt64();
                    deviceIds.push_back(deviceId);
                }
                auto worker = _workers[workerId];
                auto available = worker->availableDevices();
                vector<AudioDevice> devices;
                remove_copy_if(available.begin(), available.end(), back_inserter(devices), [&](const AudioDevice &device){
                    return find(deviceIds.begin(),deviceIds.end(), device.deviceId()) == deviceIds.end();
                });
                success.insert(success.end(),devices.begin(), devices.end());
                worker->setDevices(devices);
            }
        }
        if (success.empty()) {
            auto available = availableDevices();
            if (!available.empty()) {
                vector<AudioDevice> d;
                d.push_back(available[0]);
                _workers[0]->setDevices(d);
            }
        }
        
        // must set _activeWorkers and start them this way
        setDevices(devices());
    }

    vector<AudioDevice> method availableDevices() const
    {
        vector<AudioDevice> ret;
        for (auto &e : _workers) {
            auto sub = e->availableDevices();
            ret.insert(ret.end(), sub.begin(), sub.end());
        }
        return ret;
    }
    
    void method setDevices(const vector<AudioDevice> &devices)
    {
        vector<shared_ptr<IPlaybackWorker>> oldWorkers = _activeWorkers;
        
        std::map<shared_ptr<IPlaybackWorker>, vector<AudioDevice>> targetDevices;
        for (auto &w : _workers) {
            // instantianate map key-value pairs
            targetDevices[w];
        }
        
        for (auto &device : devices) {
            auto it = targetDevices.begin();
            for (; it != targetDevices.end() ; ++it) {
                if (it->first == device.target()) {
                    break;
                }
            }
            if (it == targetDevices.end()) {
                continue;
            }
            if (it == targetDevices.end()) {
                continue;
            }
            it->second.push_back(device);
        }
        
        vector<shared_ptr<IPlaybackWorker>> newWorkers;
        Json::Value preference = Json::arrayValue;
        for (auto &w : _workers) {
            auto d = targetDevices[w];

            Json::Value ids = Json::arrayValue;
            for (const auto &device : d) {
                ids.append(device.deviceId());
            }
            preference.append(ids);

            w->setDevices(d);
            if (d.empty()) {
                w->stop(true);
            } else {
                if (_playing) {
                    w->play();
                }
                newWorkers.push_back(w);
            }
        }
        Json::FastWriter writer;
        IApp::instance()->preferences().setStringForKey(kKeyDevices, writer.write(preference));
        
        _activeWorkers = newWorkers;
    }

    vector<AudioDevice> method devices()
    {
        vector<AudioDevice> ret;
        for (auto &w : _workers) {
            auto dev = w->devices();
            ret.insert(ret.end(), dev.begin(), dev.end());
        }
        return ret;
    }
    
    void method play()
    {
        if (!_playing) {
            // clear buffers (test by seeking merlin's gun from middle to beginining)
            for (int i = 0 ; i < kBufferNum ; ++i) {
                _buffers[i].consumeAll();
            }
        }
        _playing = true;
        vector<shared_ptr<IPlaybackWorker>> workers = _activeWorkers;
        for (auto &w : workers) {
            w->play();
        }
    }
    
    void method stop(bool release)
    {
        _playing = false;
        vector<shared_ptr<IPlaybackWorker>> workers = _activeWorkers;
        for (auto &w : workers) {
            w->stop(release);
        }
    }
    
    void method setVolume(float volume)
    {
        // chomrecast has a lower cap on volume...
        //volume = volume * volume * volume * volume;

        // not only active workers
        vector<shared_ptr<IPlaybackWorker>> workers = _workers;
        for (auto &w : workers) {
            w->setVolume(volume);
        }
    }
    
    IPlaybackWorker::PcmFormat method pcmFormat()
    {
        // only float is supported now
        return PcmFormat::Float32;
    }
    
    int method getUncompressedDataInto(char *dataPtr, int dataSize, uint8_t bufferId)
    {
#ifdef DEBUG
        //Logger::stream() << "get audio data: " << (int)bufferId << " thread: " << std::this_thread::get_id();
#endif
#ifdef UNION_SINGLEOUTPUT
        int got = PlaybackController::instance()->getUncompressedDataInto(dataPtr, dataSize,0);
        processEq(dataPtr, got);
        return got;
#endif
        
        _active[bufferId] = true;
        CircularBuffer &buf = _buffers[bufferId];
        int32_t available;
        void *tail = buf.tail(available);
        if (tail) {
            if (available > dataSize) {
                available = dataSize;
            }
            memcpy(dataPtr, tail, available);
            buf.consume(available);

            debugStreamAppend(std::string("played") + std::to_string(bufferId), dataPtr, available);

            return available;
        } 
        return 0;
    }

    void method silence(uint8_t bufferId)
    {
        _active[bufferId] = false;
        _buffers[bufferId].consumeAll();
    }

    bool method resetVolume()
    {
        bool ret = false;
        for (auto &w : _workers) {
            if (w->resetVolume()) {
                ret = true;
            }
        }
        return ret;
    }

    void method terminate()
    {
        vector<shared_ptr<IPlaybackWorker>> workers = _activeWorkers;
        for (auto &w : workers) {
            w->terminate();
        }
    }
}
