//
//  PlaybackItem.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/28/12.
//
//

#include <iostream>
#include <fstream>
#include "Environment.h"
#include "PlaybackItem.h"
#include "CircularBuffer.h"
#include "DecoderMpg123.h"
#include "DecoderAv.h"
#include "DecoderMpg123.h"
#include "DecoderPcm.h"
#include "MainExecutor.h"
#include "IApp.h"
#include "IPreferences.h"
#include "DecoderAv.h"

namespace Gear
{
#define method PlaybackItem::
    
#ifdef TIZEN
    static const int kSampleSize = 2 * 2; // buffer size: 16-bit int stereo
#else
    static const int kSampleSize = 4 * 2; // buffer size: 32-float stereo
#endif
    static const int kFrequency = 44100;
    static const int kCompressedBlockSize = 256;
    static const int kOneSecondInBuffer = kSampleSize * kFrequency;
    
    // TODO: this is just debug value, 1 sec is normal
    static int kCompressedMaximumBufferSize;
    
    shared_ptr<PlaybackItem> method create(const shared_ptr<IPlaybackData> &aData)
    {
        shared_ptr<PlaybackItem> ret(new PlaybackItem(aData));
        ret->waitForData();
        return ret;
    }
    
    static int oneSecond()
    {
        // needed at initialization
        if (kCompressedMaximumBufferSize == 0) {
            unsigned long bufferSizeMillisecs = IApp::instance()->preferences().uintForKey("BufferSizeMillisecs");
            if (bufferSizeMillisecs == 0) {
                bufferSizeMillisecs = 1000;
                // we might need a bigger buffer, especially on iphone 4
#if TARGET_OS_IPHONE
//                bufferSizeMillisecs *= 6;
                bufferSizeMillisecs *= 2;
#endif
            }
            kCompressedMaximumBufferSize = ((float)bufferSizeMillisecs)/1000.0f *kOneSecondInBuffer;
        }
        return kCompressedMaximumBufferSize;
    }

    static shared_ptr<IDecoder> createDecoder(const shared_ptr<const IPlaybackData> &data)
    {
        auto format = IPlaybackData::Format::PcmSignedInt16;
        if (data) {
            format = data->format();
        }
        switch(format) {
            case IPlaybackData::Format::Mp3:
                return shared_ptr<IDecoder>(new Gear::DecoderMpg123());
            case IPlaybackData::Format::Mp4:
#ifdef DISABLE_DECODERAV
                return shared_ptr<IDecoder>(new Gear::DecoderMpg123());
#else
                return DecoderAv::create();
#endif
            case IPlaybackData::Format::PcmSignedInt16:
                return DecoderPcm::create();
        }
    }
    
    method PlaybackItem(const shared_ptr<IPlaybackData> &data) :
        _data(data),
        _uncompressedUnusedData(oneSecond()),
        _initialBuffering(true),
        _currentTime(0.0f),
        _finished(!data),
        _reportFinished(_finished),
        _compressedDataPosition(0),
        _downloading(true),
        _gapBytesMax(IApp::instance()->preferences().uintForKey("ForceGapMilliseconds")/1000.0f * kOneSecondInBuffer),
        //_gapBytesMax(5*kOneSecondInBuffer),
        _gapBytes(0),
#ifdef DISABLE_DECODER_ALL
		_decoder(DecoderPcm::create())
#elif defined(DISABLE_DECODERAV)
		// simulator does not have libav
		_decoder(new Gear::DecoderMpg123())
#else
		_decoder(createDecoder(data))
#endif
    {
    }
    
    void method setDelegate(const shared_ptr<PlaybackItemDelegate> &delegate)
    {
        _delegate = delegate;
    }
    
    
    int method getUncompressedDataInto(char *buffer, int bufferLength)
    {
        if (!_data) {
            return 0;
        }
        
        int32_t availableBytes;
        void *readBuffer = _uncompressedUnusedData.tail(availableBytes);
        //std::cout << "available: " << availableBytes << std::endl;
        
        if (availableBytes == 0 && !_finished) {
            _initialBuffering = true;
            // this is necessary for restarting song:
            auto pThis = shared_from_this();
            _executor.addTask([pThis]{
                pThis->processData();
            });
            return 0;
        }
        // we must think of the very short songs
        if (_finished) {
            // a special request by ismon was to include gaps between songs. quite strange idea imho...
            if (_gapBytes >= _gapBytesMax) {
                // prevent situation where PlaybackController knows that we are finished and prematuraly halts
                _reportFinished = _finished;
            } else {
                _gapBytes += bufferLength;
                memset(buffer, 0, bufferLength);
            }
        }

        if (_initialBuffering && !_finished) {
            //std::cout << "still initial buffering, finished:" << _finished << "\n";
            if (availableBytes >= kCompressedMaximumBufferSize) {
                _initialBuffering = false;
            } else {
            	// we really need this prebuffering on rare occasions
				auto pThis = shared_from_this();
            	_executor.addTask([pThis]{
					pThis->processData();
				});
                return 0;
            }
        }
        
        int totalAmount = 0;
        while (readBuffer != NULL && bufferLength > 0) {
            int amount = bufferLength < availableBytes ? bufferLength : availableBytes;
            totalAmount += amount;
            memcpy(buffer, readBuffer, amount);
            buffer += amount;
            bufferLength -= amount;
            
            _uncompressedUnusedData.consume(amount);
            
            /*FILE *file = fopen("./decoded.pcm", "a");
             fwrite(buffer, 1, amount, file);
             fclose(file);
             */
            
            _currentTime += ((float)amount) / kOneSecondInBuffer;
            
            readBuffer = _uncompressedUnusedData.tail(availableBytes);
        }

        if (_finished && !_reportFinished) {
            int amount = bufferLength;
            totalAmount += amount;

            memset(buffer, 0, amount);
            buffer+= amount;
            bufferLength -= amount;
        }
        
        //dispatch_async would slow us down.
        auto pThis = shared_from_this();
        _executor.addTask([pThis]{
            pThis->processData();
        });
        
        /*if (OSAtomicTestAndSet(0, &lock) == 0) {
         [self processData];
         OSAtomicTestAndClear(0, &lock);
         }*/
        
        return totalAmount;
    }
    
    
    bool method isSongFinished()
    {
        return _reportFinished;
    }

    void method processData()
    {
        if (!_data) {
            return;
        }
        
        if (_data->failed()) {
            // encountered problems, which led us to 100% cpu and stopped playback
            
            _finished = true;
            return;
        }
        
        //NSAssert(dispatch_get_current_queue() == queue, @"wrong queue");
        int32_t outSize;
        void *outData;
        bool finishing = false;
        // has anything happened?
        bool processed = false;
        bool leftOver = false;
        
        auto totalLength = _data->totalLength();
        if (totalLength != 0 && _compressedDataPosition >= totalLength) {
            
            processed = true;
			// this is wrong since there may be leftover data in the decoder!
            //_finished = true;
        	leftOver = true;
        	while ((outData = _uncompressedUnusedData.head(outSize)) != NULL) {
        		//IDecoder::Status error;
				size_t producedBytes = 0;
        		//error = _
                _decoder->readInto((unsigned char *)outData, outSize, producedBytes);
				_uncompressedUnusedData.produce(producedBytes);
				// NeedMore is irrelevant since actually there is no more!
				// producedbytes is more trustworthy at this stage...
        		if (producedBytes == 0) {
        			_finished = true;
        			break;
        		}
        	}

        } else {
        	while (true) {
                auto available = _data->offsetAvailable((int)_compressedDataPosition);
                outData = _uncompressedUnusedData.head(outSize);
                if (!(available > 0 && (outData != NULL))) {
                    break;
                }

                processed = true;
				bool returnNeeded = false;
				_data->accessChunk((int)_compressedDataPosition, [&](const char *inData, int inSize){
					
					if (_compressedDataPosition == 0) {
						_decoder->setTotalLength(_data->totalLength());
					}
					finishing = false;

					if (inSize < kCompressedBlockSize) {

						if (!_downloading) {
							// downloading ceased, feed all remaining
							finishing = true;
						} else {
							// still downloading, wait for next
							returnNeeded = true;;
						}
					} else {
						_finished = false;
					}


					size_t producedBytes = 0;

					Gear::IDecoder::Status error;
					// this is wrong! only supply more data if the decoder specifically asks for it!
                    //if (!finishing) {
						error = _decoder->readInto((unsigned char *)outData, outSize, producedBytes);
					//} else {
					//	error = IDecoder::Status::NeedMore;
					//}
                    
                    if (error == IDecoder::Status::NoError && producedBytes == 0) {
                        // somehow we stuck in an endless loop, this is to fix that
                        returnNeeded = true;
                    }

					//AppLog("ffmpeg got data: %d", inSize);

					while (error == IDecoder::Status::NeedMore && inSize > 0) {

						// note: kDecoderEnd may also be interesting
						if (producedBytes > 0) {
							/*FILE *outHandle = fopen("/tmp/decoded.raw", "ab");
							fwrite(outData, producedBytes, 1, outHandle);
							fclose(outHandle);*/

							_uncompressedUnusedData.produce((int)producedBytes);
							outData = _uncompressedUnusedData.head(outSize);

							//AppLog("ffmpeg space to fill: %d", outSize);
							if (outData == NULL) {
								returnNeeded = true;
							}
						}


						int advance;
						if (inSize > kCompressedBlockSize) {
							advance = kCompressedBlockSize;
						} else {
							advance = inSize;
						}
						/*FILE *handle = fopen("/tmp/original.mp3", "ab");
						fwrite(inData, advance, 1, handle);
						fclose(handle);*/

						//NSLog(@"advance: %d", advance);
						_decoder->feedFrom((const unsigned char *)inData, advance);
						inData += advance;
						inSize -= advance;
						_compressedDataPosition += advance;
						producedBytes = 0;
						error = _decoder->readInto((unsigned char *)outData, outSize, producedBytes);

						//NSLog(@"advanced: %d pos: %ld produced: %ld", advance, compressedDataPosition, producedBytes);
					}
					if (finishing) {
						// premature end?
						_finished = true;
					}

					if (error == IDecoder::Status::NoError || error == IDecoder::Status::NeedMore) {
						if (producedBytes > 0) {
							/*FILE *outHandle = fopen("/tmp/decoded.raw", "ab");
							fwrite(outData, producedBytes, 1, outHandle);
							fclose(outHandle);*/

                            //std::cout << "produce: " << producedBytes << std::endl;
							_uncompressedUnusedData.produce((int)producedBytes);
						}
					} else if (error == IDecoder::Status::End) {
						_finished = true;
					} else {
						returnNeeded = true;
					}

					//NSAssert1(status == 0, @"there should be no error decoding: %d", status);
					/*FILE *file = fopen("./encoded.mp3", "a");
					 fwrite(inData, 1, inSize, file);
					 fclose(file);
					 */

					if (_finished || finishing) {
						returnNeeded = true;
					}
				});

				if (returnNeeded) {
					break;
				}
			};
            
#ifdef DEBUG
            /*if (_data->offsetAvailable((int)_compressedDataPosition) == 0) {
                std::cout << "no more data in buffer!!!\n";
            }*/
#endif
        }
        
        if (!processed && !_downloading) {
            // make sure we bail out when not doing anything...
            
            _finished = true;
            return;
        }
        
		// don't remember exactly why this was necessary, but it seems to lead to premature end of song
        //if (_data->finished()) {
            // old: look out for gapless playback! (test!)
        	//_finished = true;
        //}
        if (!leftOver && !_finished && !finishing/* && !data->finished() */&& _uncompressedUnusedData.head(outSize) != NULL) {
            // if we bailed out, but we could use some more...
            waitForData();
        }
    }

    void method waitForData()
    {
        if (!_data) {
            return;
        }
        
        //NSLog(@"waiting async.. %ld", compressedDataPosition);
        
        auto data = _data;
        // don't retain. PlaybackController uses us, and if it does not anymore, we can be free! 
        weak_ptr<PlaybackItem> wThis = shared_from_this();
        _data->waitAsync((int)_compressedDataPosition, [wThis,data]{
            
            // if finished() is here, youtube playback halts when song is is finished() and seeked to beginning!
            //if (data->failed() || data->finished()) {
            if (data->failed()) {
                auto pThis = wThis.lock();
                if (pThis) {
                    pThis->_downloading = false;
                }
            }
            
            auto pThis = wThis.lock();
            if (pThis) {
                //dispatch_async would slow us down. (?)
                pThis->_executor.addTask([wThis]{
                    auto pThis = wThis.lock();
                    if (pThis) {
                        // this was the culprit!
                        pThis->processData();
                    }
                });
            }
            
            /*if (OSAtomicTestAndSet(0, &lock) == 0) {
             [self processData];
             OSAtomicTestAndClear(0, &lock);
             }*/
        });
    }

    float method currentTime()
    {
        return _currentTime;
    }
    
    void method setCurrentTime(float desiredTime, float ratio)
    {
        if (!_data || _data->totalLength() == 0) {
            return;
        }
        auto pThis = shared_from_this();

        _executor.addTask([pThis, desiredTime, ratio]{
            
            pThis->_finished = false;

            //off_t input_offset = [decoder seekOffset:desiredTime];
            
            // as file does not contain album art or anything, it should be fine
            off_t input_offset = pThis->_decoder->seekOffset(desiredTime, ratio * pThis->_data->totalLength());
                
            pThis->_compressedDataPosition = (int)input_offset;
#ifdef DEBUG
            std::cout << "seeking: " << input_offset << "/" << pThis->_data->totalLength() << " wish: " << desiredTime << "/" << ratio << "\n";
#endif
            
            pThis->_decoder->setDiscontinous();
            pThis->_data->seek((int)input_offset);
#ifdef DEBUG
            std::cout << "seeking after: " << input_offset << "/" << pThis->_data->totalLength() << " wish: " << desiredTime << "/" << ratio << "\n";
#endif
            
            // tried to not play remainder after pause&seek but this did not work
            pThis->_uncompressedUnusedData.consumeAll();
            pThis->_executor.addTask([pThis]{
                pThis->processData();
            });

            Base::MainExecutor::instance().addTask([pThis, desiredTime]{
                //[self httpSeek:input_offset];
                pThis->_currentTime = desiredTime;
            });
        });
    }

    bool method finishedDownloading()
    {
        if (!_data) {
            return true;
        }
        return _data->finished();
    }

    void method setRateIfPlaying(float rate)
    {
        // currently unimplemented
    }

#ifdef DEBUG
    void debugStreamAppend(const std::string &name, const char *buf, int size)
    {
        return;
        auto filename = IApp::instance()->dataPath() + "/stream-" + name + ".raw";
        std::ofstream s(filename.c_str(), std::ofstream::out | std::ofstream::app);
        s.write(buf, size);
    }
#endif

}

