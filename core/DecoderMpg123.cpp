//
//  DecoderMpg123.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "DecoderMpg123.h"
#include "IPlaybackWorker.h"
#include "IApp.h"
#include <iostream>
#include "Environment.h"
#ifndef DISABLE_DECODERAV
extern "C" {
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/mathematics.h>
}
#endif
#ifdef _WIN32
#pragma comment(lib, "lib\\win\\libmpg123-0.lib")
#pragma comment(lib, "lib\\win\\avresample.lib")
#pragma comment(lib, "lib\\win\\avutil.lib")
#endif

//#define DEBUG_SHOW_BITRATE

static const int kTargetBitrate = 44100;

namespace Gear
{
#define method DecoderMpg123::
    
    method DecoderMpg123() :
        _bitrate(0),
        _resample(nullptr)
    {
    	static bool inited = false;
    	if (!inited) {
    		inited = true;
    		mpg123_init();
    	}
    	_mpgHandle = mpg123_new(NULL, NULL);

        mpg123_param(_mpgHandle, MPG123_FLAGS, MPG123_FUZZY | MPG123_SEEKBUFFER | MPG123_GAPLESS, 0);
        mpg123_param(_mpgHandle, MPG123_INDEX_SIZE, -1, 0);
        // this is very noisy, instead we resample with libav
        //mpg123_param(_mpgHandle, MPG123_FORCE_RATE, 44100, 0);
        
        mpg123_format_none(_mpgHandle);
        
        switch(IApp::instance()->playbackWorker()->pcmFormat()) {
            case IPlaybackWorker::PcmFormat::Signed16:
                mpg123_format(_mpgHandle, 22050, MPG123_STEREO, MPG123_ENC_SIGNED_16);
                mpg123_format(_mpgHandle, 24000, MPG123_STEREO, MPG123_ENC_SIGNED_16);
                mpg123_format(_mpgHandle, 32000, MPG123_STEREO, MPG123_ENC_SIGNED_16);
                mpg123_format(_mpgHandle, 44100, MPG123_STEREO, MPG123_ENC_SIGNED_16);
                mpg123_format(_mpgHandle, 48000, MPG123_STEREO, MPG123_ENC_SIGNED_16);
                break;
            case IPlaybackWorker::PcmFormat::Float32:
                mpg123_format(_mpgHandle, 22050, MPG123_STEREO, MPG123_ENC_FLOAT_32);
                mpg123_format(_mpgHandle, 24000, MPG123_STEREO, MPG123_ENC_FLOAT_32);
                mpg123_format(_mpgHandle, 32000, MPG123_STEREO, MPG123_ENC_FLOAT_32);
                mpg123_format(_mpgHandle, 44100, MPG123_STEREO, MPG123_ENC_FLOAT_32);
                mpg123_format(_mpgHandle, 48000, MPG123_STEREO, MPG123_ENC_FLOAT_32);
                break;
        }
        mpg123_open_feed(_mpgHandle);
    }
    
    method ~DecoderMpg123()
    {
        mpg123_close(_mpgHandle);
        mpg123_delete(_mpgHandle);
        
#ifndef DISABLE_DECODERAV
        if (_resample) {
            avresample_free(&_resample);
        }
#endif
    }
    
    IDecoder::Status method readInto(unsigned char *outData, size_t outSize, size_t &producedBytes)
    {
        if (!_extraBuffer.empty()) {
            int available = _extraBuffer.size() - _extraBufferPos;
            producedBytes = available < outSize ? available : outSize;
            memcpy(outData, &_extraBuffer.at(_extraBufferPos), producedBytes);
            if (producedBytes == available) {
                _extraBuffer.clear();
            } else {
                _extraBufferPos += producedBytes;
            }
            
            return IDecoder::Status::NoError;
        }
        
        if (_bitrate == 0) {
            int channels;
            int encoding;
            mpg123_getformat(_mpgHandle, &_bitrate, &channels, &encoding);
//#ifdef DEBUG
            //std::cout << "mp3 bitrate: " << _bitrate << std::endl;
//#endif
        }

#ifndef DISABLE_DECODERAV
        if (_bitrate != 0 && _bitrate != kTargetBitrate && !_resample) {
            //std::cout << "creating resample context\n";

            _resample = avresample_alloc_context();
            av_opt_set_int(_resample, "in_channel_layout",  AV_CH_LAYOUT_STEREO,  0);
            av_opt_set_int(_resample, "out_channel_layout", AV_CH_LAYOUT_STEREO,  0);
            av_opt_set_int(_resample, "in_sample_rate",     _bitrate,             0);
            av_opt_set_int(_resample, "out_sample_rate",    kTargetBitrate,       0);
            
            switch(IApp::instance()->playbackWorker()->pcmFormat()) {
                case IPlaybackWorker::PcmFormat::Signed16:
                    av_opt_set_int(_resample, "in_sample_fmt",      AV_SAMPLE_FMT_S16,   0);
                    av_opt_set_int(_resample, "out_sample_fmt",     AV_SAMPLE_FMT_S16,   0);
                    break;
                case IPlaybackWorker::PcmFormat::Float32:
                    av_opt_set_int(_resample, "in_sample_fmt",      AV_SAMPLE_FMT_FLT,   0);
                    av_opt_set_int(_resample, "out_sample_fmt",     AV_SAMPLE_FMT_FLT,   0);
                    break;
            }
            avresample_open(_resample);
        }
#endif
        
        int error = 0;
        if (_resample) {
#ifndef DISABLE_DECODERAV
            int sampleSize;
            switch(IApp::instance()->playbackWorker()->pcmFormat()) {
                case IPlaybackWorker::PcmFormat::Signed16:
                    sampleSize = 2;
                    break;
                case IPlaybackWorker::PcmFormat::Float32:
                    sampleSize = sizeof(float);
                    break;
            }
            sampleSize *= 2; // stereo
            
            // want to reverse this: we know the output buffer size at target bitrate, how much input do we need?
            //int out_samples = avresample_available(_resample) +
            //  av_rescale_rnd(avresample_get_delay(_resample) + in_samples, kTargetBitrate, _bitrate, AV_ROUND_UP);
            
            static const int kExtraSize = 256;
            
            unsigned char *useBuffer = outData;
            int out_samples;
            if (outSize >= kExtraSize) {
                out_samples = outSize / sampleSize;
            } else {
                _extraBuffer.resize(kExtraSize);
                useBuffer = &_extraBuffer.front();
                out_samples = kExtraSize / sampleSize;
            }
            auto avAvailable = avresample_available(_resample);
            auto avDelay = avresample_get_delay(_resample);
            int in_samples = av_rescale_rnd(out_samples - avAvailable - avDelay, _bitrate, kTargetBitrate, AV_ROUND_DOWN);
            if (in_samples < 0) {
                in_samples = 0;
            }
            
            size_t producedMpg = 0;
            _resampleData.resize(in_samples * sampleSize);
            error = mpg123_read(_mpgHandle, &_resampleData.front(), in_samples * sampleSize, &producedMpg);
        
            uint8_t *output[] = {useBuffer};
            uint8_t *input[] = {&_resampleData.front()};
            //std::cout << "convert " << outSize << "," << out_samples << " " << producedMpg << "," << producedMpg/sampleSize << "," << in_samples * sampleSize << std::endl;
            
            out_samples = avresample_convert(_resample, output, out_samples * sampleSize, out_samples, input, producedMpg, producedMpg / sampleSize);
            producedBytes = out_samples * sampleSize;
            
            if (useBuffer != outData) {
                _extraBuffer.resize(producedBytes);
                int reallyProduced = (producedBytes < outSize) ? producedBytes : outSize;
                memcpy(outData, useBuffer, reallyProduced);
                _extraBufferPos = reallyProduced;
                if (reallyProduced == producedBytes) {
                    _extraBuffer.clear();
                } else {
                    // use next occasion
                }
                producedBytes = reallyProduced;
            }
            
            if (producedBytes == 0) {
                //std::cout << "not good " << out_samples * sampleSize << "   usebuf: " << (useBuffer != outData) << std::endl;;
            }
#endif
        } else {
            error = mpg123_read(_mpgHandle, outData, outSize, &producedBytes);
        }
        
        switch(error) {
            case 0: {
                /*         long val;
                 double fval;
                 int ret = mpg123_getparam(mpgHandle, MPG123_FLAGS, &val, &fval);
                 int gapless = (val & MPG123_GAPLESS) == MPG123_GAPLESS;
                 */
                
#ifdef DEBUG_SHOW_BITRATE
                static int counter;
                if (++counter % 50 == 0) {
                    mpg123_frameinfo info;
                    mpg123_info(_mpgHandle, &info);
                    std::cout << "bitrate: " << info.bitrate << " target: " << info.abr_rate << " mode: " << (info.vbr == MPG123_CBR ? "CBR" : (info.vbr == MPG123_VBR ? "VBR" : "ABR")) << std::endl;
                }
#endif
                
                    
                return IDecoder::Status::NoError;
            }
            case MPG123_NEED_MORE:
                return IDecoder::Status::NeedMore;
            case MPG123_DONE:
                return IDecoder::Status::End;
            default:
                return IDecoder::Status::UnknownError;
        }
    }
    
    void method feedFrom(unsigned const char *inData, size_t inSize)
    {
        mpg123_feed(_mpgHandle, inData, inSize);
    }
    
    long method seekOffset(float time, size_t guess)
    {
        off_t input_offset;
        if (mpg123_feedseek(_mpgHandle, time * 44100, SEEK_SET, &input_offset) > 0) {
            
            std::cout << "seek position: " << time << " seconds " << input_offset << " bytes" << std::endl;
            return input_offset;
        }
        return guess;
    }
    
    void method setDiscontinous()
    {
    	// this does not help seeking buffer problem:

    	/*unsigned char buf[8192];
    	// empty buffer
    	for (;;) {
    		size_t produced = 0;
    		mpg123_read(_mpgHandle, buf, sizeof(buf), &produced);
    		if (produced == 0) {
    			break;
    		}
    	}
		*/

    	//_started = false;
        // nada
    }
}
