//
//  PlaybackWorker.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/8/13.
//
//

#include <iostream>
#include <sstream>
#include "PlaybackWorker.h"
#import "PlaybackWorkerImpl.h"
#include "Environment.h"
#if TARGET_OS_IPHONE
#include <AVFoundation/AVFoundation.h>
#else
#include "StringUtility.h"
#endif
#include LOCK_GUARD_H

namespace Gear
{
#define method PlaybackWorker::

    using THREAD_NS::lock_guard;
    using THREAD_NS::mutex;
    
    shared_ptr<PlaybackWorker> method create()
    {
        auto ret = shared_ptr<PlaybackWorker>(new PlaybackWorker());
        ret->impl = (void *)CFBridgingRetain([[PlaybackWorkerImpl alloc] initWithWorker:ret]);
        return ret;
    }
    
    method PlaybackWorker() :
        _multiDevice(0)
    {
    }
    
    void method setController(const shared_ptr<PlaybackController> &controller)
    {
        IPlaybackWorker::setController(controller);
        [(__bridge PlaybackWorkerImpl *)impl setController:controller];
    }
    
    method ~PlaybackWorker()
    {
        CFBridgingRelease(impl);
    }
    
    void method play()
    {
        [(__bridge PlaybackWorkerImpl *)impl play];
    }
    
    void method stop(bool release)
    {
        [(__bridge PlaybackWorkerImpl *)impl stop:release];
    }
    
#if TARGET_OS_MACDESKTOP
    bool method resetVolume()
    {
        return [(__bridge PlaybackWorkerImpl *)impl resetVolume];
    }
    
    void method selectDeviceId(unsigned long long deviceId)
    {
        [(__bridge PlaybackWorkerImpl *)impl selectDeviceId:deviceId];
    }
#endif

    void method setDevices(const vector<AudioDevice> &aDevices)
    {
        vector<AudioDevice> selectedDevices = aDevices;

#if TARGET_OS_MACDESKTOP
        // some preoprocessing is needed (filter out multiple airplay targets)
        vector<uint32_t> deviceIds;
        bool shouldAddDefault = false;
        selectedDevices.erase(remove_if(selectedDevices.begin(), selectedDevices.end(), [&](const AudioDevice &device){
            auto deviceId = device.deviceId() & 0xFFFFFFFF;
            if (deviceId == 0) {
                shouldAddDefault = true;
                return false;
            }

            if (find(deviceIds.begin(), deviceIds.end(), deviceId) == deviceIds.end()) {
                deviceIds.push_back(deviceId);
                return false;
            } else {
                return true;
            }
        }), selectedDevices.end());

        // set the source here!
        //std::cout << "selected devices count: " << selectedDevices.size() << std::endl;
        for (const auto &device : selectedDevices) {
            auto deviceId = device.deviceId() & 0xFFFFFFFF;
            UInt32 sourceId = device.deviceId() >> 32;
            //std::cout << "selected device: " << deviceId << " / " << sourceId << std::endl;

            //UInt32 sourceIdArr[] = {3,1};
            //status = AudioObjectSetPropertyData(64, &sourceAddress, 0, NULL, sizeof(sourceIdArr), sourceIdArr);
            if (sourceId != 0) {
                AudioObjectPropertyAddress sourceAddress;
                sourceAddress.mScope = kAudioObjectPropertyScopeOutput;
                sourceAddress.mElement = kAudioObjectPropertyElementMaster;
                sourceAddress.mSelector = kAudioDevicePropertyDataSource;
                OSStatus status = 0;
                UInt32 oldSource = 0;
                UInt32 oldSourceSize = sizeof(oldSource);
                AudioObjectGetPropertyData(deviceId, &sourceAddress, 0, NULL, &oldSourceSize, &oldSource);
                if (oldSource != sourceId) {

                    status = AudioObjectSetPropertyData(deviceId, &sourceAddress, 0, NULL, sizeof(UInt32), &sourceId);
                    if (status != 0) {
                        NSLog(@"error selecting output source");
                    } else {
        //                NSLog(@"selected output source");
                    }
                }
            }

            if (selectedDevices.size() > 1) {
                // manually set sample rate, otherwise it will be quirky
                AudioObjectPropertyAddress address;
                address.mScope = kAudioObjectPropertyScopeOutput;
                address.mElement = kAudioObjectPropertyElementMaster;
                address.mSelector = kAudioDevicePropertyNominalSampleRate;
                Float64 sr = 0;
                UInt32 size = sizeof(sr);
                AudioObjectGetPropertyData(deviceId, &address, 0, 0, &size, &sr);
                const Float64 desired = 44100;
                if (sr < desired-100 || sr > desired + 100) {
                    // if we don't set the sample rate in a multi-output configuration, there can 
                    // be helicopter sound according to steve falcon
                    AudioObjectSetPropertyData(deviceId, &address, 0, 0, sizeof(desired), &desired);
                }
            }
        }

        if (selectedDevices.size() == 0) {
            stop(true);
        } else if (selectedDevices.size() == 1){
            selectDeviceId(selectedDevices.front().deviceId());
        } else {
            if (shouldAddDefault) {
                auto defaultDeviceId = [impl defaultDeviceId];
                if (find(deviceIds.begin(), deviceIds.end(), defaultDeviceId) == deviceIds.end()) {
                    deviceIds.push_back(defaultDeviceId);
                }
            }

            BOOL playing = [impl playing];
            if (playing) {
                [impl stop:YES];
            }

            IPlaybackWorker::setDevices(selectedDevices);

            createAggregate();
            setAggregateDevices(deviceIds);
            selectDeviceId(_multiDevice);
            if (playing) {
                [impl play];
            }
        }
#endif
        IPlaybackWorker::setDevices(selectedDevices);
    }

    uint32_t method multiDevice() const
    {
        return _multiDevice;
    }    

    std::vector<SInt32> method multiChannelMap() const
    {
        lock_guard<mutex> l(_multiChannelMapMutex);
        return _multiChannelMap;
    }
    
    void method setVolume(float volume)
    {
        [(__bridge PlaybackWorkerImpl *)impl setVolume:volume];
    }
    
    AudioStreamBasicDescription method usedPCMFormat()
    {
        return [PlaybackWorkerImpl usedPCMFormat];
    }
    
    Gear::IPlaybackWorker::PcmFormat method pcmFormat()
    {
        return PcmFormat::Float32;
    }
    
    
    
    
    // borrowed from AudioDeviceManager
    
    //#define TEST_AGGREGATE

    static string aggregateName()
    {
#ifdef TEST_AGGREGATE
        return "G-Ear aggregate testdevice";
#else
        return "G-Ear aggregate device";
#endif
    }
    
    vector<AudioDevice> method availableDevices() const
    {
        vector<AudioDevice> ret;
#if TARGET_OS_IPHONE
        ret.push_back(AudioDevice(0, "Device Output", shared_from_this()));
        return ret;
#else
        ret.push_back(AudioDevice(0, "Default Output", shared_from_this()));

        AudioObjectPropertyAddress  propertyAddress;
        AudioObjectID               *deviceIDs;
        UInt32                      propertySize;
        int                         numDevices;
        
        propertyAddress.mSelector = kAudioHardwarePropertyDevices;
        propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
        propertyAddress.mElement = kAudioObjectPropertyElementMaster;
        if (AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize) == noErr) {
            numDevices = propertySize / sizeof(AudioDeviceID);
            deviceIDs = (AudioDeviceID *)calloc(numDevices, sizeof(AudioDeviceID));
            
            if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize, deviceIDs) == noErr) {
                AudioObjectPropertyAddress      deviceAddress;
                char                            deviceName[256];
                
                for (int idx=0 ; idx<numDevices ; idx++) {
                    
                    propertySize = sizeof(deviceName);
                    deviceAddress.mSelector = kAudioDevicePropertyDeviceName;
                    deviceAddress.mScope = kAudioObjectPropertyScopeGlobal;
                    deviceAddress.mElement = kAudioObjectPropertyElementMaster;
                    
                    if (AudioObjectGetPropertyData(deviceIDs[idx], &deviceAddress, 0, NULL, &propertySize, deviceName) == noErr) {
                        
                        if (deviceName != aggregateName()) {
                            deviceAddress.mSelector = kAudioDevicePropertyStreams;
                            deviceAddress.mScope = kAudioObjectPropertyScopeOutput;
                            deviceAddress.mElement = kAudioObjectPropertyElementMaster;
                            UInt32 dataSize = 0;
                            OSStatus status = AudioObjectGetPropertyDataSize(deviceIDs[idx],                                                                                &deviceAddress,
                                                                             0,
                                                                             NULL,
                                                                             &dataSize);
                            UInt32 streamCount = dataSize / sizeof(AudioStreamID);
                            if (streamCount < 1) {
                                continue;
                            }
                        }
                        
                        AudioDeviceID deviceId = deviceIDs[idx];
                        
                        UInt32 sourceIds[128];
                        
                        UInt32 size = sizeof(sourceIds);
                        
                        AudioObjectPropertyAddress sourceAddress;
                        sourceAddress.mScope = kAudioObjectPropertyScopeOutput;
                        sourceAddress.mElement = kAudioObjectPropertyElementMaster;
                        sourceAddress.mSelector = kAudioDevicePropertyDataSources;
                        
                        OSStatus result = AudioObjectGetPropertyData(deviceId, &sourceAddress, 0, NULL, &size, sourceIds);
                        int numSources = size/sizeof(UInt32);
                        if (result == 0 && numSources > 0) {
                            
                            AudioObjectPropertyAddress sourceNameAddress;
                            sourceNameAddress.mScope = kAudioObjectPropertyScopeOutput;
                            sourceNameAddress.mElement = kAudioObjectPropertyElementMaster;
                            sourceNameAddress.mSelector = kAudioDevicePropertyDataSourceNameForIDCFString;
                            
                            std::cout << "device (" << deviceId << ") " << deviceName << " count: " << numSources << std::endl;
                            for (int i = 0 ; i < numSources ; i++) {
                                UInt32 sourceId = sourceIds[i];
                                
                                AudioValueTranslation translation;
                                translation.mInputData = &sourceId;
                                translation.mInputDataSize = sizeof(sourceId);
                                CFStringRef sourceNameCf = NULL;
                                translation.mOutputData = &sourceNameCf;
                                translation.mOutputDataSize = sizeof(sourceNameCf);
                                UInt32 translationSize = sizeof(translation);
                                
                                result = AudioObjectGetPropertyData(deviceId, &sourceNameAddress, 0, NULL, &translationSize, &translation);
                                if (result == 0) {
                                    string sourceName = convertString((__bridge NSString *)sourceNameCf);
                                    std::cout << "source: " << sourceName << " " << sourceId <<std::endl;
                                    CFRelease(sourceNameCf);
                                    std::stringstream compositeName;
                                    compositeName << deviceName << " (" << sourceName << ")";
                                    
                                    ret.push_back(AudioDevice( ((uint64_t)deviceId) | (((uint64_t) sourceId) << 32), compositeName.str(), shared_from_this()));
                                }
                                
                            }
                        } else {
#ifdef TEST_AGGREGATE_SHOWIT
                            ret.push_back(AudioDevice(deviceId, deviceName, shared_from_this()));
                            if (deviceName == aggregateName()) {
                                _multiDevice = deviceId;
                            }
#else
                            if (deviceName != aggregateName()) {
                                ret.push_back(AudioDevice(deviceId, deviceName, shared_from_this()));
                            } else {
                                _multiDevice = deviceId;
                            }
#endif
                        }
                    }
                }
            }
            
            free(deviceIDs);
        }

        // TODO: skip aggregate device
        
        return ret;
#endif
    }
    
   /* void method setSources(const vector<uint32_t> &sources)
    {
        lock_guard<mutex> l(_sourcesMutex);
        _sources = sources;
    }
    
    vector<uint32_t> method sources() const
    {
        lock_guard<mutex> l(_sourcesMutex);
        return _sources;
    }*/
    

#if !TARGET_OS_IPHONE
    OSStatus method createAggregate()
    {
        if (_multiDevice != 0) {
            // not necessarry
            return 0;
        }

        // extremely undocumented...
        // based on http://daveaddey.com/?p=51
        
        UInt32 outSize;
        Boolean outWritable;
        
        //-----------------------
        // Start to create a new aggregate by getting the base audio hardware plugin
        //-----------------------
        
        OSStatus osErr = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyPlugInForBundleID, &outSize, &outWritable);
        if (osErr != noErr) return osErr;
        
        AudioValueTranslation pluginAVT;
        
        CFStringRef inBundleRef = CFSTR("com.apple.audio.CoreAudio");
        AudioObjectID pluginID;
        
        pluginAVT.mInputData = &inBundleRef;
        pluginAVT.mInputDataSize = sizeof(inBundleRef);
        pluginAVT.mOutputData = &pluginID;
        pluginAVT.mOutputDataSize = sizeof(pluginID);
        
        osErr = AudioHardwareGetProperty(kAudioHardwarePropertyPlugInForBundleID, &outSize, &pluginAVT);
        if (osErr != noErr) return osErr;
        
        //-----------------------
        // Feed the dictionary to the plugin, to create a blank aggregate device
        //-----------------------
        
        AudioObjectPropertyAddress pluginAOPA;
        pluginAOPA.mSelector = kAudioPlugInCreateAggregateDevice;
        pluginAOPA.mScope = kAudioObjectPropertyScopeGlobal;
        pluginAOPA.mElement = kAudioObjectPropertyElementMaster;
        UInt32 outDataSize;
        
        osErr = AudioObjectGetPropertyDataSize(pluginID, &pluginAOPA, 0, NULL, &outDataSize);
        if (osErr != noErr) return osErr;
        
        AudioDeviceID outAggregateDevice;
        
        //-----------------------
        // Create a CFDictionary for our aggregate device
        //-----------------------
        
        CFMutableDictionaryRef aggDeviceDict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        
        CFStringRef AggregateDeviceNameRef = (__bridge CFStringRef) convertString(aggregateName());
#ifdef TEST_AGGREGATE
        CFStringRef AggregateDeviceUIDRef = CFSTR("com.treasurebox.gear.aggregatetest");
#else
        CFStringRef AggregateDeviceUIDRef = CFSTR("com.treasurebox.gear.aggregate");
#endif

        // add the name of the device to the dictionary
        CFDictionaryAddValue(aggDeviceDict, CFSTR(kAudioAggregateDeviceNameKey), AggregateDeviceNameRef);
        
        // add our choice of UID for the aggregate device to the dictionary
        CFDictionaryAddValue(aggDeviceDict, CFSTR(kAudioAggregateDeviceUIDKey), AggregateDeviceUIDRef);
        
        // this does not work with older os x versions
#ifndef TEST_AGGREGATE
        CFDictionaryAddValue(aggDeviceDict, CFSTR(kAudioAggregateDeviceIsPrivateKey), kCFBooleanTrue);
#endif

        osErr = AudioObjectGetPropertyData(pluginID, &pluginAOPA, sizeof(aggDeviceDict), &aggDeviceDict, &outDataSize, &outAggregateDevice);
        CFRelease(aggDeviceDict);
        
        if (osErr != noErr) return osErr;
        
        _multiDevice = outAggregateDevice;
        return noErr;
    }


    static NSString *deviceUidForId(AudioObjectID deviceId)
    {
        UInt32          propertySize;
        CFStringRef     uidString = 0;
        
        propertySize = sizeof(uidString);
        AudioObjectPropertyAddress      deviceAddress;
        deviceAddress.mSelector = kAudioDevicePropertyDeviceUID;
        deviceAddress.mScope = kAudioObjectPropertyScopeGlobal;
        deviceAddress.mElement = kAudioObjectPropertyElementMaster;
        if (AudioObjectGetPropertyData(deviceId, &deviceAddress, 0, NULL, &propertySize, &uidString) == noErr) {
            
            return CFBridgingRelease(uidString);
        }
        return nil;
    }

    OSStatus method setAggregateDevices(const std::vector<uint32_t> &deviceIds)
    {
        //return 0;
        OSStatus osErr = noErr;
        UInt32 outDataSize;
        
        //-----------------------
        // Create a CFMutableArray for our sub-device list
        //-----------------------
        
        // this example assumes that you already know the UID of the device to be added
        // you can find this for a given AudioDeviceID via AudioDeviceGetProperty for the kAudioDevicePropertyDeviceUID property
        // obviously the example deviceUID below won't actually work!
        //CFStringRef deviceUID = CFSTR("UIDOfDeviceToBeAdded");
        
        // we need to append the UID for each device to a CFMutableArray, so create one here
        CFMutableArrayRef subDevicesArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
        
        // just the one sub-device in this example, so append the sub-device's UID to the CFArray
        NSMutableArray *devicesPresent = [[NSMutableArray alloc] init];
        vector<uint32_t> sourcesPresent;

        //NSString *clockDevice = nil;
        std::vector<SInt32> channelMap;
        int num = 0;

        for (const uint32_t device : deviceIds) {
            //uint64_t combined = [deviceId longLongValue];
            //uint32_t device = combined & 0xFFFFFFFF;
            //uint32_t source = (combined >> 32) & 0xFFFFFFFF;
            id deviceObj = @(device);
            if (![devicesPresent containsObject:deviceObj]) {
                NSString *deviceUid = deviceUidForId(device);
                if (deviceUid == nil) {
                    continue;
                }
                AudioObjectPropertyAddress address;
                address.mScope = kAudioObjectPropertyScopeOutput;
                address.mElement = kAudioObjectPropertyElementMaster;
                address.mSelector = kAudioDevicePropertyStreamConfiguration;
                UInt32 datasize;
                osErr = AudioObjectGetPropertyDataSize(device, &address, 0, NULL, &datasize);
                if (osErr != noErr) {
                    continue;
                }
                char pBufferList[datasize];
                AudioBufferList &bufferList = *(reinterpret_cast<AudioBufferList *>(pBufferList));
                osErr = AudioObjectGetPropertyData(device, &address, 0, NULL, &datasize, &bufferList);
                if (osErr != noErr || bufferList.mNumberBuffers == 0) {
                    continue;
                }
                int localChannelCounter = 0;
                for (int i = 0 ; i < bufferList.mNumberBuffers ; ++i) {
                    auto channelPerBuffer = bufferList.mBuffers[i].mNumberChannels;
                    num += channelPerBuffer;
                    for (int k = 0 ; k < channelPerBuffer ; ++k) {
                        if (localChannelCounter < 2) {
                            channelMap.push_back(localChannelCounter);
                        } else {
                            channelMap.push_back(-1);
                        }
                        ++localChannelCounter;
                    }
                }

                CFArrayAppendValue(subDevicesArray, (__bridge CFStringRef)deviceUid);
                [devicesPresent addObject:deviceObj];


                //clockDevice = deviceUid;
            }
            /*if (source > 0 && find(sourcesPresent.begin(),sourcesPresent.end(),source) == sourcesPresent.end()) {
                sourcesPresent.push_back(source);
            }*/
        }

        {
            lock_guard<mutex> l(_multiChannelMapMutex);
            _multiChannelMap = channelMap;
        }
//#define LOG_AGGREGATE
#ifdef LOG_AGGREGATE
        NSLog(@"aggregate device total number of channels: %d", num);
        NSMutableArray *arr = [NSMutableArray array];
        for (auto &val : channelMap) {
            [arr addObject:@(val)];
        }
        NSLog(@"aggregate device channel map: %@", arr);
#endif                
        
        //-----------------------
        // Set the sub-device list
        //-----------------------
        
        AudioObjectPropertyAddress pluginAOPA;
        pluginAOPA.mSelector = kAudioAggregateDevicePropertyFullSubDeviceList;
        pluginAOPA.mScope = kAudioObjectPropertyScopeGlobal;
        pluginAOPA.mElement = kAudioObjectPropertyElementMaster;
        outDataSize = sizeof(CFMutableArrayRef);
        osErr = AudioObjectSetPropertyData(_multiDevice, &pluginAOPA, 0, NULL, outDataSize, &subDevicesArray);
        if (osErr != noErr) return osErr;
        
        //-----------------------
        // Set the master device
        //-----------------------
        
        // set the master device manually (this is the device which will act as the master clock for the aggregate device)
        // pass in the UID of the device you want to use
        /*
         pluginAOPA.mSelector = kAudioAggregateDevicePropertyMasterSubDevice;
         pluginAOPA.mScope = kAudioObjectPropertyScopeGlobal;
         pluginAOPA.mElement = kAudioObjectPropertyElementMaster;
         outDataSize = sizeof(clockDevice);
         osErr = AudioObjectSetPropertyData(_multiDevice, &pluginAOPA, 0, NULL, outDataSize, &clockDevice);
         if (osErr != noErr) return osErr;
         */

        //-----------------------
        // Clean up
        //-----------------------
        
        NSLog(@"set devices for aggregate: (%d) %@", (int)_multiDevice, (__bridge id) subDevicesArray);
        
        // release the CF objects we have created - we don't need them any more
        CFRelease(subDevicesArray);
        
        //AudioDeviceManager::instance().setSources(sourcesPresent);
        
        return noErr;
    }
#endif
    
}