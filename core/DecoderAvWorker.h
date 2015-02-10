//
//  DecoderAvWorker.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 25/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__DecoderAvWorker__
#define __G_Ear_core__DecoderAvWorker__

#include "stdplus.h"
#include "CircularBuffer.h"
#include "IDecoder.h"

#define __STDC_CONSTANT_MACROS
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}
#include MUTEX_H
#include CONDITION_VARIABLE_H
#include SHAREDFROMTHIS_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
    class DecoderAvWorker final : public MEMORY_NS::enable_shared_from_this<DecoderAvWorker>
    {
    public:
        static shared_ptr<DecoderAvWorker> create();
        
        void start();
        void release();
        void setTotalLength(int64_t totalLength);
        void feedFrom(unsigned const char *inData, size_t inSize);
        IDecoder::Status readInto(unsigned char *outData, size_t outSize, size_t &producedBytes);
        long seekOffset(float time, size_t guess);
        
        virtual ~DecoderAvWorker();
        
    private:
        DecoderAvWorker();
        
        void work();
        bool finishing();
        
        bool parseHeader();
		bool decodeFrame();
		void freeFrame();
        void seekFinished();
        
        enum class Command
        {
            Nothing,
            Seek,
            Halt
        };
        
        void issueCommand(Command cmd);
        
		static int read_packet(void *opaque, uint8_t *buf, int buf_size);
		static int64_t seek_packet(void *opaque, int64_t offset, int whence);
        
        int audioStreamId;
		AVCodecContext *codecContext;
		AVFormatContext *formatContext;
		AVIOContext *ioContext;
		unsigned char *avBuffer;
        
        float maxFloat;
		float maxFloatReciprocal;
		//SwrContext *convertContext;
        
		AVFrame *audioFrame;
		CircularBuffer convertedAudioData;
        
		bool packetInited;
		AVPacket packet;
        
        void doSeek();
        
        ATOMIC_NS::atomic<Command> _command;
        ATOMIC_NS::atomic<float> _seekTo;
        ATOMIC_NS::atomic<long> _totalLength;
        
		int64_t currentPosition;
        THREAD_NS::mutex _commandMutex;
        THREAD_NS::condition_variable _commandVariable;
    
        CircularBuffer ioBuffer;
    };
}

#endif /* defined(__G_Ear_core__DecoderAvWorker__) */
