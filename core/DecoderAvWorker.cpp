//
//  DecoderAvWorker.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 25/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include "DecoderAvWorker.h"
#include "BackgroundExecutor.h"
#include "IPlaybackWorker.h"
#include "IApp.h"

#ifdef _WIN32
#pragma comment(lib, "lib\\win\\avcodec.lib")
#pragma comment(lib, "lib\\win\\avformat.lib")
#pragma comment(lib, "lib\\win\\avresample.lib")
#pragma comment(lib, "lib\\win\\avutil.lib")
#endif

namespace Gear
{
    using THREAD_NS::mutex;
    using THREAD_NS::lock_guard;
    using THREAD_NS::unique_lock;

    // kBuffesize must be at least kChunksize + 256
	static int kBufferSize = 65536;
    static int kChunkSize = 32768;
    
#define method DecoderAvWorker::
    
    static void printerr(int result)
    {
        char str[1024];
        av_strerror(result, str, sizeof(str));
        std::cout << "averror: " << std::string(str) << std::endl;
    }
    
	static int initLibAvCodec()
	{
		avcodec_register_all();
		av_register_all();
        
		// very important, otherwise logging takes a _lot_ of time
		av_log_set_level(0);
        
		return 0;
	}
    
    shared_ptr<DecoderAvWorker> method create()
    {
        return shared_ptr<DecoderAvWorker>(new DecoderAvWorker());
    }
    
    
    method DecoderAvWorker() :
        ioBuffer(kBufferSize),
        convertedAudioData(65536),
        packetInited(false),
        maxFloat(1.0f),
        maxFloatReciprocal(1.0f),
        currentPosition(0)
    {
        static int inited = initLibAvCodec();
		inited = 0;
        _totalLength = 0;
        
        _command = Command::Nothing;
        
        audioFrame = nullptr;
        
		codecContext = nullptr;
		// plus padding
		avBuffer = (unsigned char *)av_malloc(kBufferSize + 8);
		ioContext = avio_alloc_context(avBuffer, kBufferSize, 0, this, &read_packet, nullptr, &seek_packet);
		ioContext->seekable = 0;
		formatContext = nullptr;
    }
    
    method ~DecoderAvWorker()
    {
		if (packetInited) {
			av_free_packet(&packet);
		}
		freeFrame();
		if (codecContext) {
			avcodec_close(codecContext);
		}
		if (formatContext) {
			avformat_close_input(&formatContext);
		}
		if (ioContext) {
			// workaround...
			av_free(ioContext->buffer);
			av_free(ioContext);
		}
        
        if (audioFrame) {
            avcodec_free_frame(&audioFrame);
        }
		//if (avBuffer) {
		//	av_free(avBuffer);
		//}
    }
    
    void method release()
    {
        issueCommand(Command::Halt);
    }
    
    void method issueCommand(Command cmd)
    {
        _command = cmd;
        THREAD_NS::lock_guard<mutex> l(_commandMutex);
        _commandVariable.notify_all();
    }
    
    void method start()
    {
        auto pThis = shared_from_this();
        BackgroundExecutor::instance().addTask([pThis]{
            pThis->work();
        });
    }
    
    void method work()
    {
        //std::cout << "WORK trying header\n";
        
        if (!parseHeader()) {
            //std::cout << "WORK parsed header fail\n";
            return;
        }
        
        //std::cout << "WORK parsed header\n";
        
        for (;;) {
            Command command = _command;
            bool wait = false;
            switch(command) {
                case Command::Seek:
                    doSeek();
                    // don't break, continue decoding from new position!
                    //break;
                case Command::Nothing: {
                    
                    int32_t bufferSpace = 0;
                    convertedAudioData.headTotal(bufferSpace);
                    if (bufferSpace >= 32768) {
                    
                        int32_t available;
                        bool success;
                        success = decodeFrame();
                        if (command == Command::Seek) {
                            continue;
                        }
                        convertedAudioData.tail(available);
                        
                        //std::cout << "WORK after decode: free " << available << " success " << success << std::endl;
                        
                        if (success && available == 0) {
                            wait = false;
                        } else {
                            wait = true;
                        }
                    } else {
                        wait = true;
                    }
                    break;
                }
                case Command::Halt:
                    return;
            }
            if (wait) {
                unique_lock<mutex> l(_commandMutex);
                _commandVariable.wait_for(l, CHRONO_NS::milliseconds(200));
            }
        }
    }
    
    void method feedFrom(unsigned const char *inData, size_t inSize)
	{
		// two steps because of wrapping...
		/*int debugTotal = 0;
		ioBuffer.headTotal(debugTotal);
        std::cout << "ffmpeg new data: " << inSize << " available space: " << debugTotal << std::endl;
        */
        //std::cout << "WORKi feed: " << inSize << std::endl;
        
		if (inSize > 0) {
			int freeBytes = 0;
			void *freeSpace = ioBuffer.head(freeBytes);
            
			int toCopy = (inSize < freeBytes) ? inSize : freeBytes;
			memcpy(freeSpace, inData, toCopy);
			ioBuffer.produce(toCopy);
            
			inData += toCopy;
			inSize -= toCopy;
		}
		if (inSize > 0) {
			int freeBytes = 0;
			void *freeSpace = ioBuffer.head(freeBytes);
            
			int toCopy = (inSize < freeBytes) ? inSize : freeBytes;
			memcpy(freeSpace, inData, toCopy);
			ioBuffer.produce(toCopy);
            
			inSize -= toCopy;
		}
		if (inSize > 0) {
            std::cout << "WORKi: GREAT PROBLEM " << inSize << std::endl;
		}
        
		//int occupied = 0;
		//ioBuffer.tail(occupied);
		//AppLog("got new bytes: %d (free before: %d occupied: %d)", inSize, freeBytes, occupied);
        
        {
            lock_guard<mutex> l(_commandMutex);
            _commandVariable.notify_all();
        }
	}
    
    IDecoder::Status method readInto(unsigned char *outData, size_t outSize, size_t &producedBytes)
	{
        if (_command == Command::Halt) {
            return IDecoder::Status::End;
        }
        
        producedBytes = 0;
        
		while (outSize > 0) {
			int available = 0;
			void *data = convertedAudioData.tail(available);
			if (available > 0) {
				if (available > outSize) {
					available = outSize;
				}
				memcpy(outData, data, available);
				convertedAudioData.consume(available);
				producedBytes += available;
				outData += available;
				outSize -= available;
			} else {
				break;
			}
		}
        
        if (producedBytes == 0) {
            int available = 0;
            ioBuffer.headTotal(available);
            if (available >= 256) {
                return IDecoder::Status::NeedMore;
            } else {
                return IDecoder::Status::NoError;
            }
        }
        //std::cout << "WORKi produced: " << producedBytes << std::endl;
        
        {
            lock_guard<mutex> l(_commandMutex);
            _commandVariable.notify_all();
        }
        
        return IDecoder::Status::NoError;
	}
    
    void method seekFinished()
    {
        ioContext->seekable = 0;
        
        //std::cout << "WORKER seeking done" << std::endl;
        _command = Command::Nothing;
        {
            lock_guard<mutex> l(_commandMutex);
            _commandVariable.notify_all();
        }
    }
    
    int method read_packet(void *opaque, uint8_t *buf, int buf_size)
	{
		DecoderAvWorker *decoder = reinterpret_cast<DecoderAvWorker *>(opaque);
        
#ifdef DEBUG
		if (buf_size > 32768) {
            //std::cout<< "ffmpeg reading " << buf_size << std::endl;
		}
#endif
        if (decoder->_command == Command::Seek) {
            decoder->seekFinished();
        }
        
        
		int totalBufSize = buf_size;
        
        while (buf_size > 0) {
            int available = 0;
            void *data = decoder->ioBuffer.tail(available);
            if (available == 0) {
                
                if (decoder->_command == Command::Seek) {
                    memset(buf, 0, buf_size);
                    return totalBufSize;
                }
                
                {
                    unique_lock<mutex> l(decoder->_commandMutex);
                    decoder->_commandVariable.wait(l);
                }
                continue;
            }
            
            if (available > buf_size) {
                available = buf_size;
            }
            memcpy(buf, data, available);
            decoder->ioBuffer.consume(available);
            buf += available;
            buf_size -= available;
        }
        
        //if (buf_size != 0) {
        //    std::cout << "WORK not enough data\n";
        //}
        
        auto count = totalBufSize - buf_size;
		decoder->currentPosition += count;
        
		return count;
	}
    
    void method setTotalLength(int64_t totalLength)
    {
        _totalLength = totalLength;
    }
    
    bool method finishing()
	{
		int available = 0;
		ioBuffer.tailTotal(available);
        
		return currentPosition + available >= _totalLength;
	}

    // returns true if getting more data might make sense
	bool method decodeFrame()
	{
		//AppLog("ffmpeg av_read_frame begin");
        
		int available = 0;
		ioBuffer.tailTotal(available);
        
		bool outOfData = false;
		//if (available >= kChunkSize || (available > 0 && finishing())) {
			int result = av_read_frame(formatContext, &packet);
            if (_command == Command::Seek) {
                if (result >= 0) {
                    av_free_packet(&packet);
                }
                // hopefully this will not do harm...
                seekFinished();
                
                return true;
            }
            
            //std::cout << "ffmpeg av_read_frame " << result << "/" << packet.stream_index << std::endl;
			if (result < 0) {
#ifdef DEBUG
                printerr(result);
#endif
				//AppLog("ffmpeg readframe failed");
				return false;
			}
            
			if (packet.stream_index != audioStreamId) {
                av_free_packet(&packet);
				return true;
			}
		
    /*} else {
			packet.data = NULL;
			packet.size = 0;
			outOfData = true;
		}*/
        
        
		int got_frame_ptr = 0;
		//AppLog("ffmpeg stream index: %d size: %d flags: %d", packet.stream_index, packet.size, packet.flags);
		//AppLog("ffmpeg decode_audio begin");
        //std::cout << "packet size: " << packet.size << std::endl;
        
        if (!audioFrame) {
            audioFrame = avcodec_alloc_frame();
        } else {
            avcodec_get_frame_defaults(audioFrame);
        }
        
        
		// int len =
        avcodec_decode_audio4(codecContext, audioFrame, &got_frame_ptr, &packet);
        //std::cout << "decoded len: " << len << "/" << codecContext->sample_fmt << " float planar: " << AV_SAMPLE_FMT_FLTP << std::endl;
        if (packet.data == NULL) {
			//AppLog("ffmpeg flushing, got: %d", len);
		}
		//AppLog("ffmpeg decode_audio end..  %d %d %d real size: %d returned len: %d packet: %d diff: %d", packet.stream_index, got_frame_ptr, audioFrame->linesize[0], audioFrame->nb_samples, len, packet.size, packet.size-len);
        
		if (got_frame_ptr > 0) {

            
#ifdef _DEBUG
			//AppLog("ffmpeg sampleformat: %d", codecContext->sample_fmt);
#endif
            
			int floatSize = audioFrame->linesize[0] / 2;
            //std::cout << "linesize " << floatSize << std::endl;
			uint8_t *data = audioFrame->data[0];
			uint8_t *dataRight = audioFrame->data[1];
			if (!dataRight) {
				dataRight = data;
			}
			uint8_t *dataEnd = data + floatSize;
			float *dataFloat = (float *)data;
			float *dataRightFloat = (float *)dataRight;
            
			while(dataFloat < ((float *)dataEnd)) {
				int convertedDataSpace = 0;
				void *convertedAudioBuf = convertedAudioData.head(convertedDataSpace);
                if (convertedAudioBuf == nullptr) {
                    break;
                }
				void *convertedAudioBufEnd = ((char *)convertedAudioBuf) + convertedDataSpace;
				//static const float ditherAmplitude = 1.0f/32767.0f /2.0f;
				//static int ditherTick = 0;
                
                //static float phase = 0;
                
                switch(IApp::instance()->playbackWorker()->pcmFormat()) {
                    case IPlaybackWorker::PcmFormat::Signed16:{
                        
                        int16_t *convertedData = (int16_t *)convertedAudioBuf;
                        for (; dataFloat < ((float *)dataEnd) && convertedData < (int16_t *)convertedAudioBufEnd;) {
                            // dithering missing!
                            //float dither = ((ditherTick % 4) / 2) ? (+ditherAmplitude) : (-ditherAmplitude);
                            const float dither = 0.0f;
                            if (*dataFloat > maxFloat) {
                                maxFloat = *dataFloat;
                                maxFloatReciprocal = 1.0f/maxFloat;
                            }
                            if (*dataFloat < -maxFloat) {
                                maxFloat = -*dataFloat;
                                maxFloatReciprocal = 1.0f/maxFloat;
                            }
                            if (*dataRightFloat > maxFloat) {
                                maxFloat = *dataRightFloat;
                                maxFloatReciprocal = 1.0f/maxFloat;
                            }
                            if (*dataRightFloat < -maxFloat) {
                                maxFloat = -*dataRightFloat;
                                maxFloatReciprocal = 1.0f/maxFloat;
                            }
                            
                            /*	float f = sin(phase) * 32767.0f;
                             int16_t ii = (int16_t) f;
                             *convertedData = ii;
                             ++convertedData;
                             *convertedData = ii;
                             ++convertedData;
                             phase += (2*M_PI*440 * 1.0f/44100.0f);
                             */
                            
                            *convertedData = (int16_t) ((*dataFloat +dither) * 32767 * maxFloatReciprocal);
                            ++convertedData;
                            *convertedData = (int16_t) ((*dataRightFloat +dither) * 32767 * maxFloatReciprocal);
                            ++convertedData;
                            
                            ++dataFloat;
                            ++dataRightFloat;
                            //++ditherTick;
                        }
                        
                        int amount = ((char *)convertedData) - ((char *)convertedAudioBuf);
                        convertedAudioData.produce(amount);
                        break;
                    }
                        //default:{
                    case IPlaybackWorker::PcmFormat::Float32: {
                        float *convertedData = (float *)convertedAudioBuf;
                        for (; dataFloat < ((float *)dataEnd) && convertedData < (float *)convertedAudioBufEnd;) {
                            
                            //std::cout << "data " << *dataFloat << std::endl;
                            //int16_t *ii = (int16_t *)dataFloat;
                            //int32_t *uu = (int32_t *)dataFloat;
                            *convertedData = *dataFloat;
                            ++convertedData;
                            
                            //std::cout << "warning mono\n";
                            // TODO: only mono for now!
                            *convertedData = *dataRightFloat;
                            //*convertedData = *dataFloat;
                            ++convertedData;
                            
                            
                            /*float f = sin(phase);
                             *convertedData = f;
                             ++convertedData;
                             *convertedData = f;
                             ++convertedData;
                             phase += (2*M_PI*440 * 1.0f/44100.0f);
                             */
                            
                            
                            ++dataFloat;
                            ++dataRightFloat;
                        }
                        
                        int amount = ((char *)convertedData) - ((char *)convertedAudioBuf);
                        convertedAudioData.produce(amount);
                        break;
                    }
                }
                
				//AppLog("ffmpeg put buffer: %d  (%d samples) max float: %f", amount, 0, maxFloat);
			}
			//AppLog("ffmpeg put ends here. original freq: %d channels: %d",
			//			codecContext->sample_rate, codecContext->channels);
			//av_audio_convert_free(convert);

            
			//AppLog("ffmpeg converting to int end");
            
            av_free_packet(&packet);
			freeFrame();
			return true;
		}
        
        av_free_packet(&packet);
		freeFrame();
		return !outOfData;
	}
    
	void method freeFrame()
	{
        // tizen is the only platform (now) which is using ffmpeg, otherwise we are moving on to libav
#ifdef TIZEN
		if (codecContext && codecContext->refcounted_frames == 1) {
			av_frame_unref(audioFrame);
		}
#else
        // av_frame_unref(audioFrame);
#endif
	}
    
	bool method parseHeader()
	{
		formatContext = avformat_alloc_context();
		formatContext->pb = ioContext;
        
		int result = avformat_open_input(&formatContext, "dummy.filename", NULL, NULL);
        
		if (result < 0) {
			return false;
		}
        
		result = avformat_find_stream_info(formatContext, nullptr);
		if (result < 0) {
			return false;
		}
        
		audioStreamId = -1;
        
		// To find the first audio stream.
		int i;
		for (i = 0; i < (int)formatContext->nb_streams; i++) {
			if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
				audioStreamId = i;
                //std::cout << "got audio stream" << i << std::endl;
				break;
			}
		}
        
		if (audioStreamId == -1) {
			return false;
		}
        
		codecContext = formatContext->streams[audioStreamId]->codec;
		codecContext->request_sample_fmt = AV_SAMPLE_FMT_FLTP;
		codecContext->request_channels = 2;
		AVCodec* codec = avcodec_find_decoder(codecContext->codec_id);
		if (!codec) {
			return false;
		}
		result = avcodec_open2(codecContext, codec, nullptr);
		if (result < 0) {
			return false;
		}
        
		av_init_packet(&packet);
		packet.data = NULL;
		packet.size = 0;
		packet.stream_index = audioStreamId;
		packetInited = true;
        
		return true;
	}
    
    long method seekOffset(float time, size_t guess)
	{
		if (!formatContext) {
			return 0;
		}
        
        long totalLength = _totalLength;
        
		// don't seek too close to the end...
		if (totalLength > 0 && time > 0 && guess > totalLength - 2000000) {
			int oneSecInBytes = totalLength / time;
			float tooCloseBySecs = ((totalLength-guess))/oneSecInBytes;
			//AppLog("ffmpeg too close to end %f", tooCloseBySecs);
			static const int kTooClose = 30;
			if (tooCloseBySecs < kTooClose) {
				time -= (kTooClose - tooCloseBySecs);
				if (time < 0) {
					time = 0;
				}
			}
		}
        
        _seekTo = time;
        issueCommand(Command::Seek);
        
        {
            unique_lock<mutex> l(_commandMutex);
            while (_command == Command::Seek) {
                _commandVariable.wait_for(l, CHRONO_NS::milliseconds(500));
            }
        }
        
        return currentPosition;
	}
    
    int64_t method seek_packet(void *opaque, int64_t offset, int whence)
	{
		DecoderAvWorker *decoder = reinterpret_cast<DecoderAvWorker *>(opaque);
		if (whence == AVSEEK_SIZE) {
			long totalSize = decoder->_totalLength;
			if (totalSize < 1000) {
				// return some bogus value, 0 would cause segfault
				return 14771838;
			}
			//AppLog("ffmpeg returning totalsize of %d", totalSize);
			return totalSize;
		}
        
        //std::cout << "WORKER seek to " << offset << std::endl;
        decoder->currentPosition = offset;
		return 0;
	}
    
    void method doSeek()
    {
        //std::cout << "WORK doseek\n";
        
        ioBuffer.consumeAll();
        convertedAudioData.consumeAll();
        
        ioContext->seekable = 1;
        
        float time = _seekTo;
		int64_t target = time * AV_TIME_BASE;
		av_seek_frame(formatContext, -1, target, 0);
        
        // seeking will be triggered next time av_read_frame is called
        /*int result = av_read_frame(formatContext, &packet);
        if (result >= 0) {
            av_free_packet(&packet);
        }
        
        ioContext->seekable = 0;
        
        _command = Command::Nothing;
        {
            lock_guard<mutex> l(_commandMutex);
            _commandVariable.notify_all();
        }*/
    }
}
