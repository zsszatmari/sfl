//
//  PlaybackWorker.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 11/13/12.
//
//

#include <iostream>
#import "PlaybackWorkerImpl.h"
#import "PlaybackController.h"
#if !TARGET_OS_IPHONE
#include <CoreAudio/AudioHardware.h>
#endif
#include <vector>
#include "PlaybackController.h"
#include "PlaybackWorker.h"
#include "IAudioGenerator.h"
#include "IApp.h"
#include "IPreferences.h"
#include "json.h"

using std::vector;
using namespace Gear;

#if !TARGET_OS_IPHONE

@interface PlaybackWorkerHelper : NSObject
@end

@implementation PlaybackWorkerHelper

- (id)sharedDelegate
{
    return nil;
}

- (id)mainWindowController
{
    return nil;
}

- (id)volumeSlider
{
    return nil;
}

- (void)setFloatValue:(float)value
{
}

@end

#endif


@interface PlaybackWorkerImpl () {
    AUGraph graph;
    //AudioUnit mixerUnit;
    AudioUnit converterUnit;
    AudioUnit outputUnit;
    AudioStreamBasicDescription currentFormat;
    BOOL playing;
    BOOL eqConnected;
    
    AUNode converterNode;
    AUNode outputNode;

    float lastVolume;
    
#if !TARGET_OS_IPHONE
    AudioUnit eqUnit;
    AUNode eqNode;
    AudioDeviceID selectedDevice;
    AudioDeviceID reallySelectedDevice;
#endif
}
@end

@implementation PlaybackWorkerImpl {
    ATOMIC_NS::atomic<bool> _silence;
    shared_ptr<PlaybackWorker> _worker;
}

@synthesize controller;

static OSStatus renderCallback(void *							inRefCon,
AudioUnitRenderActionFlags *	ioActionFlags,
const AudioTimeStamp *			inTimeStamp,
UInt32							inBusNumber,
UInt32							inNumberFrames,
AudioBufferList *				ioData)
{
    /*@autoreleasepool {
        [[NSThread currentThread] setThreadPriority:1.0f];
    }*/
    
    
        PlaybackWorkerImpl *worker = (__bridge PlaybackWorkerImpl *)inRefCon;
        /*if (worker->_silence == true) {
            *ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
            return 0;
        }*/
        int bytesReceived = 0;
        int bytesNeeded = -1;
        if (ioData->mNumberBuffers == 1) {
            AudioBuffer *buffer = &ioData->mBuffers[0];
            
            bytesNeeded = buffer->mDataByteSize;
            //auto now = std::chrono::steady_clock::now();
            bytesReceived = worker->_worker->generator()->getUncompressedDataInto((char *)buffer->mData, bytesNeeded,1);
            
            
            // these can be useful for testing purposes
            // but remember: sluggish playback is caused by running from xcode!
            /*static float phase = 0;
            bytesReceived = bytesNeeded;
            float *data = (float *)buffer->mData;
            for (int i = 0 ; i < bytesNeeded/sizeof(float) ;) {
                data[i] = sin(phase);
                data[i+1] = sin(phase);
                
                phase += 2*M_PI * 440 * (1.0f/44100.0f);
                i+=2;
            }*/
            /*int16_t *data = (int16_t *)buffer->mData;
            for (int i = 0 ; i < bytesNeeded/sizeof(int16_t) ;) {
                data[i] = sin(phase) * 32767;
                data[i+1] = sin(phase) * 32767;
                
                phase += 2*M_PI * 440 * (1.0f/44100.0f);
                //i+=1;
                i+=2;
            }*/
            
            //int diff = duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count();
            buffer->mDataByteSize = bytesReceived;
            
/*#ifdef DEBUG
            if (diff > 1000 || bytesReceived < bytesNeeded) {
                printf("data for continous playback %d/%d %d microsecs\n", bytesReceived, bytesNeeded, diff);
            }
#endif*/
            
        } else {
            // just signal that we need format spec asap
            worker->_worker->generator()->getUncompressedDataInto(nullptr, 0, 1);
        }
        
        if (bytesReceived != bytesNeeded) {
#if DEBUG
            if (bytesReceived > 0) {
                std::cout << "less bytes than asked for " << bytesNeeded << " / " << bytesReceived << std::endl;
            }
#endif
            if (bytesReceived > 0) {
                return bytesReceived;
            }
            *ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
            return 0;
        }
        
        return 0;
    //}
}


- (id)initWithWorker:(const shared_ptr<PlaybackWorker> &)worker
{
    self = [super init];
    if (self) {
        _worker = worker;
        _silence = false;
        lastVolume = -1;
        
        OSStatus result = NewAUGraph(&graph);
        if (result != 0) {
            return nil;
        }
        
        AudioComponentDescription converter_description;
        converter_description.componentType = kAudioUnitType_FormatConverter;
        converter_description.componentSubType = kAudioUnitSubType_AUConverter;
        converter_description.componentFlags = 0;
        converter_description.componentFlagsMask = 0;
        converter_description.componentManufacturer = kAudioUnitManufacturer_Apple;
        result = AUGraphAddNode(graph, &converter_description, &converterNode);
        NSAssert(result == 0, @"au init converter");
        
#if !TARGET_OS_IPHONE
        AudioComponentDescription eq_description;
        eq_description.componentType = kAudioUnitType_Effect;
        eq_description.componentSubType = kAudioUnitSubType_GraphicEQ;
        eq_description.componentFlags = 0;
        eq_description.componentFlagsMask = 0;
        eq_description.componentManufacturer = kAudioUnitManufacturer_Apple;
        result = AUGraphAddNode(graph, &eq_description, &eqNode);
        NSAssert(result == 0, @"eq");
#endif
        
        /*AUNode mixerNode;
        AudioComponentDescription mixer_desc;
        mixer_desc.componentType = kAudioUnitType_Mixer;
        mixer_desc.componentSubType = kAudioUnitSubType_StereoMixer;
        mixer_desc.componentFlags = 0;
        mixer_desc.componentFlagsMask = 0;
        mixer_desc.componentManufacturer = kAudioUnitManufacturer_Apple;
        result = AUGraphAddNode(graph, &mixer_desc, &mixerNode);
        NSAssert(result == 0, @"au init mixer");
        */
        
        // "In addition to the audio converter and channel mapping capabilities, you can specify the device to connect to by setting the kAudioOutputUnitProperty_CurrentDevice property to the ID of an AudioDevice object in the HAL. " type: AudioDeviceID
        AudioComponentDescription output_desc;
        output_desc.componentType = kAudioUnitType_Output;
#if TARGET_OS_IPHONE
        output_desc.componentSubType = kAudioUnitSubType_RemoteIO;
#else 
        output_desc.componentSubType = kAudioUnitSubType_HALOutput;
#endif
        output_desc.componentFlags = 0;
        output_desc.componentFlagsMask = 0;
        output_desc.componentManufacturer = kAudioUnitManufacturer_Apple;
        result = AUGraphAddNode(graph, &output_desc, &outputNode);
        NSAssert(result == 0, @"au init output");
        
        
        //result = AUGraphConnectNodeInput(graph, mixerNode, 0, outputNode, 0);
        NSAssert(result == 0, @"connect1");
        //result = AUGraphConnectNodeInput(graph, converterNode, 0, mixerNode, 0);
        result = AUGraphConnectNodeInput(graph, converterNode, 0, outputNode, 0);
        NSAssert(result == 0, @"connect2");
        
        result = AUGraphOpen(graph);
        NSAssert(result == 0, @"graphopen");
        
        
        //result = AUGraphNodeInfo(graph, mixerNode, NULL, &mixerUnit);
        //NSAssert(result == 0, @"mixernode");
        result = AUGraphNodeInfo(graph, converterNode, NULL, &converterUnit);
        NSAssert(result == 0, @"converternode");
        result = AUGraphNodeInfo(graph, outputNode, NULL, &outputUnit);
        NSAssert(result == 0, @"outputnode");
#if !TARGET_OS_IPHONE
        result = AUGraphNodeInfo(graph, eqNode, NULL, &eqUnit);
        NSAssert(result == 0, @"eqnode");
#endif
        
        
#if !TARGET_OS_IPHONE
        AudioUnitParameterID numberOfBandsId = 10000;
        // set up a 10-channel eq
        float desired = 0; // means 10 instead of 31
        result = AudioUnitSetParameter(eqUnit, numberOfBandsId, kAudioUnitScope_Global, 0, desired, 0);
        NSAssert(result == 0, @"bands");
        
        // scopes: kAudioUnitScope_Global,kAudioUnitScope_Input, kAudioUnitScope_Output
        AudioUnitElement element = 0;
        AudioUnitParameterID params[1000];
        AudioUnitParameterID *pParams = params;
        UInt32 size = sizeof(params);
        result = AudioUnitGetProperty(eqUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global, element, pParams, &size);
        for (int i = 0 ; i < size/sizeof(AudioUnitParameterID) ; i++) {
            AudioUnitParameterID currentId = params[i];
            
            AudioUnitParameterInfo nameinfo;
            memset(&nameinfo, 0, sizeof(nameinfo));
            uint32_t size = sizeof(nameinfo);
            result = AudioUnitGetProperty(eqUnit, kAudioUnitProperty_ParameterInfo, kAudioUnitScope_Global, currentId, &nameinfo, &size);
            NSAssert(result == 0, @"geteqprop");
            
            //NSLog(@"property %d: %@ min: %f max: %f default: %f unit: %d", currentId, (__bridge id)nameinfo.cfNameString, nameinfo.minValue, nameinfo.maxValue, nameinfo.defaultValue, nameinfo.unit);
            
            if (nameinfo.unit == kAudioUnitParameterUnit_Decibels) {
                NSString *originalName = (__bridge id)nameinfo.cfNameString;
                
                NSRange range = [originalName rangeOfString:@" "];
                if (range.location != NSNotFound) {
                    NSString *shortened = [originalName substringToIndex:range.location];
                    long number = [shortened integerValue];
                    
                    if (number >= 1000) {
                        shortened = [NSString stringWithFormat:@"%ldK", number / 1000];
                    }
                    [[NSUserDefaults standardUserDefaults] setObject:shortened forKey:[NSString stringWithFormat:@"eq%dlabel", i]];
                }
            }
            
            if ((nameinfo.flags & kAudioUnitParameterFlag_CFNameRelease) == kAudioUnitParameterFlag_CFNameRelease) {
                CFRelease(nameinfo.cfNameString);
            }
        }
        
        [self setEqValues];
        [[NSNotificationCenter defaultCenter] addObserverForName:NSUserDefaultsDidChangeNotification object:nil queue:nil usingBlock:^(NSNotification *note) {
            [self setEqValues];
        }];
#endif
        
#if TARGET_OS_IPHONE
        // fix lock problem
        // see https://developer.apple.com/library/ios/qa/qa1606/_index.html
        
        //UInt32 maxFPS = 4096;
        // 4096 is technically enough, but maybe better playback experience with a higher value
        
        UInt32 maxFPS = 8192;
        

        //maxFPS = 32768;
        result = AudioUnitSetProperty(converterUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &maxFPS, sizeof(maxFPS));
#if DEBUG
        NSLog(@"setting frames per slice result: %d", result);
#endif
#endif
        
        //UInt32 busCount = 1;
        //AudioUnitSetProperty(mixerUnit, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0, &busCount, sizeof(busCount));
        
        
        // set up things...
        AURenderCallbackStruct renderCallbackStruct;
		renderCallbackStruct.inputProc = &renderCallback;
		renderCallbackStruct.inputProcRefCon = (__bridge void *)(self);
        
        // Set a callback for the specified node's specified input
        result = AUGraphSetNodeInputCallback(graph, converterNode, 0, &renderCallbackStruct);
        NSAssert(result == 0, @"callback");

		[self setAudioFormat];        
        
        result = AUGraphInitialize(graph);
        NSAssert(result == 0, @"initialize");
        
        //AUGraphStart(graph);
        
#if !TARGET_OS_IPHONE
        [NSTimer scheduledTimerWithTimeInterval:1.0f target:self selector:@selector(checkDefaultOutput:) userInfo:nil repeats:YES];
#endif

    }
    return self;
}

#if !TARGET_OS_IPHONE
- (long long)defaultDeviceId
{
    AudioObjectPropertyAddress  propertyAddress;
    propertyAddress.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMaster;
    
    AudioObjectID deviceId;
    UInt32 propertySize = sizeof(deviceId);
    if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize, &deviceId) == noErr) {
        
        UInt32 sourceIds[1];
        UInt32 size = sizeof(sourceIds);
        
        AudioObjectPropertyAddress sourceAddress;
        sourceAddress.mScope = kAudioObjectPropertyScopeOutput;
        sourceAddress.mElement = kAudioObjectPropertyElementMaster;
        sourceAddress.mSelector = kAudioDevicePropertyDataSources;
        
        OSStatus result = AudioObjectGetPropertyData(deviceId, &sourceAddress, 0, NULL, &size, sourceIds);
        int numSources = size/sizeof(UInt32);
        if (result == 0 && numSources > 0) {
            return ((uint64_t)deviceId) | (((uint64_t)sourceIds[0]) << 32);
        }
        
        return deviceId;
    }
    
    // no default found, select the first one if possible...
    auto d = _worker->availableDevices();
    if (d.size() > 0) {
        return d.at(0).deviceId();
    }
    
    return 0;
}
#endif

#if !TARGET_OS_IPHONE
- (void)checkDefaultOutput:(NSTimer *)timer
{
    if (selectedDevice == 0) {
        auto def = [self defaultDeviceId];
        
        if (def != 0) {
            [self doSelectDeviceId:def];
        }
    }
}

- (void)setEqValues
{
#ifdef ENABLE_UNIVERSAL_EQ
    return;
#endif

#if DEBUG
//    NSLog(@"set eq");
#endif
    BOOL shouldEq = [[NSUserDefaults standardUserDefaults] boolForKey:@"eqEnabled"];
    if (shouldEq != eqConnected) {
        OSStatus result;
        
        if (playing) {
            AUGraphStop(graph);
        }
        if (!shouldEq) {
            
            result = AUGraphDisconnectNodeInput(graph, outputNode, 0);
            NSAssert(result == 0, @"disconnect1");
            result = AUGraphDisconnectNodeInput(graph, eqNode, 0);
            NSAssert(result == 0, @"disconnect2");
            
            result = AUGraphConnectNodeInput(graph, converterNode, 0, outputNode, 0);
            NSAssert(result == 0, @"connect2");
        } else {
            result = AUGraphDisconnectNodeInput(graph, outputNode, 0);
            NSAssert(result == 0, @"disconnect");
            
            result = AUGraphConnectNodeInput(graph, eqNode, 0, outputNode, 0);
            NSAssert(result == 0, @"connect2");
            result = AUGraphConnectNodeInput(graph, converterNode, 0, eqNode, 0);
            NSAssert(result == 0, @"connect1");
        }
        if (playing) {
            AUGraphStart(graph);
        }
        eqConnected = shouldEq;
#if DEBUG
//        NSLog(@"eq changed: %d", eqConnected);
#endif
    }
    
    if (eqConnected) {
        
        float eqPre = [[NSUserDefaults standardUserDefaults] floatForKey:@"eqPre"];
        
        for (int i = 0 ; i < 10 ; i++) {
            float value = [[NSUserDefaults standardUserDefaults] floatForKey:[NSString stringWithFormat:@"eq%d", i]];
            AudioUnitSetParameter(eqUnit, i, kAudioUnitScope_Global, 0, value + eqPre, 0);
        }
    }
}
#endif

- (void)dealloc
{
    AUGraphStop(graph);
    
    DisposeAUGraph(graph);
} 

- (void)play
{
    _silence = false;
    if (!playing) {
        IApp::instance()->disableAppNap();
        OSStatus status = AUGraphStart(graph);
#ifdef DEBUG
        NSLog(@"starting playback: %d %@", (int)status, [NSThread currentThread]);
#endif
        playing = YES;
    }
}

- (void)stop:(BOOL)release
{
    _silence = true;
    if (playing && release) {

        IApp::instance()->enableAppNap();

        OSStatus status = AUGraphStop(graph);
#ifdef DEBUG
        NSLog(@"pausing playback: %d %@", (int)status, [NSThread currentThread]);
#endif
        playing = NO;
    }
}

#if !TARGET_OS_IPHONE

- (BOOL)deviceVolumeControlPermitted
{
    // this was 'DisableDeviceVolumeControl' but it was the (bug!) the other way around...
    static BOOL ret = [[NSUserDefaults standardUserDefaults] boolForKey:@"DisableSoundDeviceVolumeControl"];
    return !ret;
}

- (BOOL)deviceVolumeControlBypassed
{
    // TODO: remove this in the future, nooone uses it
    return [[NSUserDefaults standardUserDefaults] boolForKey:@"BypassSoundDeviceVolumeControl"];
}

- (BOOL)resetVolume
{    
    if ([self deviceVolumeControlPermitted]) {

        AudioObjectPropertyAddress thePropertyAddress;
        thePropertyAddress.mSelector = kAudioDevicePropertyVolumeScalar;
        thePropertyAddress.mScope = kAudioDevicePropertyScopeOutput;
        thePropertyAddress.mElement = kAudioObjectPropertyElementMaster;
        
        // system volume change (see airplay)

        //std::cout << "getting volume... (dev " << reallySelectedDevice << ")\n";
        
        //NSLog(@"PW resetvolume: getting volume... (dev %d)", reallySelectedDevice);
        if (AudioObjectHasProperty(reallySelectedDevice, &thePropertyAddress)) {
            
            Float32 localVolume;
            UInt32 thePropSize = sizeof(Float32);
            
            // typically, airplay
            OSStatus result = AudioObjectGetPropertyData(reallySelectedDevice, &thePropertyAddress, 0, NULL, &thePropSize, &localVolume);
            
            if (result == 0) {
                
                lastVolume = localVolume;
                //std::cout << "got volume: " << localVolume << std::endl;
                
                // must be delayed otherwise can cause nsapplication problems, especially when selected output is soundflower 
                dispatch_async(dispatch_get_main_queue(), ^{
                    [[[[NSClassFromString(@"AppDelegate") sharedDelegate] mainWindowController] volumeSlider] setFloatValue:localVolume *100];
                });
                
                //[[[[AppDelegate sharedDelegate] mainWindowController] volumeSlider] setFloatValue:localVolume * 100];
                return YES;
            }
        }
    }
        
    return NO;
    // we don't care about hal volume (it's 1.0 after initialization), go for the remembered value!
    /*
    Float32 localVolume;
    OSStatus result = AudioUnitGetParameter(outputUnit, kHALOutputParam_Volume, kAudioUnitScope_Output, 0, &localVolume);
    
    NSLog(@"PW resetvolume: got hal volume %f %d", localVolume, result);
    if (result == 0) {
        lastVolume = localVolume;
        [[[[NSClassFromString(@"AppDelegate") sharedDelegate] mainWindowController] volumeSlider] setFloatValue:localVolume *100];
        return YES;
    }
    
    return NO;*/
}

- (void)setVolume:(float)vol
{
    BOOL found = NO;
    if ([self deviceVolumeControlPermitted]) {
        AudioObjectPropertyAddress thePropertyAddress;
        Float32 localVolume = vol;
        UInt32 thePropSize = sizeof(Float32);
        thePropertyAddress.mSelector = kAudioDevicePropertyVolumeScalar;
        thePropertyAddress.mScope = kAudioDevicePropertyScopeOutput;
        thePropertyAddress.mElement = kAudioObjectPropertyElementMaster;
        // see if the device supports volume control, if so, then get it
        
        //NSLog(@"PW setvolume: setting volume to %f (dev %d)", vol, reallySelectedDevice);
        if (AudioObjectHasProperty(reallySelectedDevice, &thePropertyAddress)) {
            
            if ([self deviceVolumeControlBypassed]) {
                found = YES;
            } else {
                // typically, airplay
                OSStatus result = AudioObjectSetPropertyData(reallySelectedDevice, &thePropertyAddress, 0, NULL, thePropSize, &localVolume);
                if (result == 0) {
                    //std::cout << "set airplay volume\n";
                    found = YES;
                }
            }
            //std::cout << "setair3\n";

            
            //NSLog(@"PW setvolume: SetPropertyData result: %d", (int)result);
        }
    }
    
    if (!found) {
        //NSLog(@"PW setvolume: SetPropertyData set HALoutput volume");
        
        OSStatus result = AudioUnitSetParameter(outputUnit, kHALOutputParam_Volume, kAudioUnitScope_Output, 0, vol, 0);
        
        if (result != 0) {
            NSLog(@"error mixer set parameter");
        }
    }
    
    lastVolume = vol;
}
#endif

+ (AudioStreamBasicDescription)usedPCMFormat
{
    AudioStreamBasicDescription format;
    format.mChannelsPerFrame = 2;
    format.mSampleRate = 44100;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mBitsPerChannel = 32;
    format.mFormatFlags = kAudioFormatFlagIsFloat;
    //format.mBitsPerChannel = 16;
    //format.mFormatFlags = kAudioFormatFlagIsSignedInteger;
    format.mFramesPerPacket = 1;
    format.mBytesPerFrame = format.mBitsPerChannel / 8 * format.mChannelsPerFrame;
    format.mBytesPerPacket = format.mFramesPerPacket * format.mBytesPerFrame;
    return format;
}

- (void)setAudioFormat
{
    AudioStreamBasicDescription format = [PlaybackWorkerImpl usedPCMFormat];
    
    OSStatus status = AudioUnitSetProperty(converterUnit,
                                           kAudioUnitProperty_StreamFormat,
                                           kAudioUnitScope_Input,
                                           0,
                                           &format,
                                           sizeof(format));
    if (status != noErr) {
        //-10868 format not supported? what the?
        
        NSLog(@"problem with inputformat %d", (int)status);
    }

}

#if !TARGET_OS_IPHONE
- (void)selectDeviceId:(unsigned long long)deviceId
{
    if (deviceId == 0) {
        [self checkDefaultOutput:nil];
    }
    
    // this would break multi-device support
    /*if (selectedDevice == deviceId) {
        return;
    }*/
    
    selectedDevice = (AudioDeviceID)deviceId;
    if (deviceId != 0) {
        [self doSelectDeviceId:deviceId];
    }
}

static OSStatus volumeListener( AudioObjectID                       inObjectID,
                                    UInt32                              inNumberAddresses,
                                    const AudioObjectPropertyAddress    inAddresses[],
                                    void *ptr)
{
    PlaybackWorkerImpl *s = (__bridge PlaybackWorkerImpl *)ptr;
    [s resetVolume];
    return 0;   
}   

- (void)doSelectDeviceId:(unsigned long long)deviceId
{
    static unsigned long long lastId = 0;
    if (deviceId == lastId && deviceId != _worker->multiDevice()) {
        return;
    }
            
    lastId = deviceId;
    
    AudioDeviceID localDeviceId = (AudioDeviceID)deviceId & 0xffffffff;
    //NSLog(@"device: %d source: %d", localDeviceId, (unsigned int)sourceId);
    
    OSStatus status = AudioUnitSetProperty(outputUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster, &localDeviceId, sizeof(localDeviceId));

    if (status != 0) {
        NSLog(@"error selecting output device %llu (%d)", deviceId, status);
    } else {
        //NSLog(@"output device %u set", localDeviceId);
    
        // this does not work on saffireLE
        /*
        AudioChannelLayout layout;
        memset(&layout, 0, sizeof(layout));
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
        AudioUnitSetProperty(outputUnit, kAudioUnitProperty_AudioChannelLayout, kAudioUnitScope_Output, kAudioObjectPropertyElementMaster, &layout, sizeof(layout));
        */


        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
            AudioStreamBasicDescription descr;
            memset(&descr, 0, sizeof(AudioStreamBasicDescription));
            UInt32 size = sizeof(descr);
            AudioUnitGetProperty(outputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, kAudioObjectPropertyElementMaster, &descr, &size);
            
            if (descr.mChannelsPerFrame > 2) {
                string channelMappingString = IApp::instance()->preferences().stringForKey("ChannelMapping");
                Json::Value channelMapping;
                if (!channelMappingString.empty()) {
                    Json::Reader parser;
                    parser.parse(channelMappingString, channelMapping);
                }
                if (_worker->multiDevice() == localDeviceId && localDeviceId != 0) { 
                    // special case is the aggragate device, we must distribute sound to all outputs
                    SInt32 channelMap[descr.mChannelsPerFrame];
                    auto desiredMap = _worker->multiChannelMap();
                    for(UInt32 i = 0 ; i < descr.mChannelsPerFrame ; i++)
                    {
                        channelMap[i] = i < desiredMap.size() ? desiredMap.at(i) : -1;
                        // this would be bad:
                        //channelMap[i] = i % 2;
                    }
                    AudioUnitSetProperty(outputUnit, kAudioOutputUnitProperty_ChannelMap, kAudioUnitScope_Output, kAudioObjectPropertyElementMaster, channelMap, sizeof(channelMap));

                } else if (!channelMapping.isArray()) {
                    SInt32 channelMap[descr.mChannelsPerFrame];
                    // Steve Falcon convinced me that the default output should be the first output
                    channelMap[0] = 0;
                    channelMap[1] = 1;
                    for(UInt32 i = 2 ; i < descr.mChannelsPerFrame ; i++)
                    {
                        channelMap[i] = -1;
                    }
                    AudioUnitSetProperty(outputUnit, kAudioOutputUnitProperty_ChannelMap, kAudioUnitScope_Output, kAudioObjectPropertyElementMaster, channelMap, sizeof(channelMap));
                } else {
                    int numChannels = descr.mChannelsPerFrame;
                    if (numChannels > channelMapping.size()) {
                        numChannels = channelMapping.size();
                    }
                    SInt32 channelMap[numChannels];
                    for(UInt32 i = 0 ; i < numChannels ; i++)
                    {
                        channelMap[i] = channelMapping[i].isInt() && channelMapping[i].asInt();
                    }
                    AudioUnitSetProperty(outputUnit, kAudioOutputUnitProperty_ChannelMap, kAudioUnitScope_Output, kAudioObjectPropertyElementMaster, channelMap, sizeof(channelMap));
                }
            }
        });
        //NSLog(@"result of setting channel map: %d", status);
    }
    
    if (reallySelectedDevice != localDeviceId)  {

        //NSLog(@"set device: %d", localDeviceId);

        AudioObjectPropertyAddress thePropertyAddress;
        thePropertyAddress.mSelector = kAudioDevicePropertyVolumeScalar;
        thePropertyAddress.mScope = kAudioDevicePropertyScopeOutput;
        thePropertyAddress.mElement = kAudioObjectPropertyElementMaster;
        AudioObjectRemovePropertyListener(reallySelectedDevice, &thePropertyAddress, &volumeListener, (__bridge void *)self);
        OSStatus status = AudioObjectAddPropertyListener(localDeviceId, &thePropertyAddress, &volumeListener, (__bridge void *)self);
        //NSLog(@"addproperty status: %d", status);

        reallySelectedDevice = localDeviceId;
        [self resetVolume];
        if (lastVolume >= 0) {
            [self setVolume:lastVolume];
        }
    }
}
#endif

- (BOOL)playing
{
    return playing;
}

/*
- (void)setAudioFormat:(AudioStreamBasicDescription)format
{
    format.mFormatFlags = kAudioFormatFlagIsSignedInteger;
    //format.mFormatFlags = kAudioFormatFlagIsSignedInteger| kAudioFormatFlagsNativeEndian | kLinearPCMFormatFlagIsNonInterleaved | (24 << kLinearPCMFormatFlagsSampleFractionShift);
    if (format.mBitsPerChannel != currentFormat.mBitsPerChannel ||
		format.mBytesPerFrame != currentFormat.mBytesPerFrame ||
		format.mChannelsPerFrame != currentFormat.mChannelsPerFrame ||
		format.mFormatFlags != currentFormat.mFormatFlags ||
		format.mFormatID != currentFormat.mFormatID ||
		format.mSampleRate != currentFormat.mSampleRate)
    {
		
        currentFormat = format;
        OSStatus status = AudioUnitSetProperty(converterUnit,
                                               kAudioUnitProperty_StreamFormat,
                                               kAudioUnitScope_Input,
                                               0,
                                               &format,
                                               sizeof(format));
        if (status != noErr) {
            //-10868 format not supported? what the?
            
            NSLog(@"problem with inputformat %d", status);
        }
   }
}
 */

@end
