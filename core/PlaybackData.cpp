//
//  PlaybackData.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/17/13.
//
//

#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "stdplus.h"
#include "PlaybackData.h"
#include "GearUtility.h"
#include "Thread.h"
#include "Logger.h"
#include "SeekableDownloader.h"
#include THREAD_H
#include WEAK_H

namespace Gear
{
    using std::cout;
    using std::stringstream;
    using THREAD_NS::this_thread::sleep_for;
    using MEMORY_NS::weak_ptr;
    
#define method PlaybackData::
    
    static const vector<shared_ptr<ISeekDownloader>> createDownloaders(const vector<string> &urls, const IDownloaderFactory &downloaderFactory)
    {
        auto &m = empty<std::map<string,string>>();
        
        vector<shared_ptr<ISeekDownloader>> downloaders;
        downloaders.reserve(urls.size());
        
        for (auto it = urls.begin() ; it != urls.end() ; ++it) {
        	const auto &url = *it;

            auto shared = SeekableDownloader::create(url, m);
            //auto shared = downloaderFactory.create(url, m);
            
            downloaders.push_back(shared);
        }
        return downloaders;
    }
    
    static const vector<long long> computeWaitTimes(const vector<string> &urls)
    {
        vector<long long> waitTimes;
        waitTimes.reserve(urls.size());
        
        long long expireZero = 0;
        long long before = 0;
        
        int index = 0;
        for (auto it = urls.begin() ; it != urls.end() ; ++it) {
                	const auto &url = *it;
            
            // get expiration time
            const string expireToken = "expire=";
            size_t expirePos = url.find(expireToken);
            long long expireTime = 0;
            if (expirePos != string::npos) {
                expirePos += expireToken.length();
                size_t expireEndPos = url.find("&", expirePos);
                if (expireEndPos != string::npos) {
                    string expires = url.substr(expirePos, expireEndPos - expirePos);
                    long long number = atoll(expires.c_str());
                    if (expireZero == 0) {
                        expireZero = number;
                    }
                    expireTime = number - expireZero;
                }
            }
            long long waitTime = expireTime - before;
            before = expireTime;
            
            if (index < 2) {
                waitTime = 0;
            }
            if (waitTime > 28) {
                waitTime = 28;
            }
            waitTimes.push_back(waitTime);
            
            //std::cout << "expire:" << expireTime << "diff: " << waitTime << std::endl;
            
            ++index;
        }
        return waitTimes;
    }
    
    static const vector<pair<int,int>> createRanges(const vector<string> &urls)
    {
        vector<pair<int,int>> ranges;
        ranges.reserve(urls.size());

        for (auto it = urls.begin() ; it != urls.end() ; ++it) {
        	const auto &url = *it;
            
            pair<int, int> range;
            range.first = 0;
            range.second = 0;
            char kRange[] = "&range=";
            size_t pos = url.find(kRange);
            if (pos != string::npos) {
                pos += sizeof(kRange) -1;
                stringstream ss(url.substr(pos));
                ss >> range.first;
                ss.ignore();
                ss >> range.second;
            }
            
            ranges.push_back(range);
        }
        return ranges;
    }
    
    void method startPrefetchingAtIndex(int index)
    {
        // schedule all for later execution because urls will expire!
        // TODO: don't use threads, expensive!
        vector<weak_ptr<IDownloader>> weakDownloaders;
        weakDownloaders.reserve(downloaders.size());
        transform(downloaders.begin(), downloaders.end(), back_inserter(weakDownloaders), [](const shared_ptr<IDownloader> &rhs){
            return weak_ptr<IDownloader>(rhs);
        });
        
        vector<long long> waitTimes = _waitTimes;
        if (index > 0) {
            size_t eraseIndex = index;
            if (eraseIndex > waitTimes.size()) {
                eraseIndex = waitTimes.size();
            }
            if (waitTimes.size() > 0) {
                // start the next one right now...
                waitTimes[0] = 0;
            }
            
            waitTimes.erase(waitTimes.begin(), waitTimes.begin() + eraseIndex);
            if (eraseIndex > weakDownloaders.size()) {
                eraseIndex = weakDownloaders.size();
            }
            weakDownloaders.erase(weakDownloaders.begin(), weakDownloaders.begin() + eraseIndex);
        }
        
        
        Base::Thread thread([weakDownloaders, waitTimes]{
            
            // must start after delay whether we are there or not...
            
            auto it = waitTimes.begin();
            
            for (auto itDownloader = weakDownloaders.begin() ; itDownloader != weakDownloaders.end() ; ++itDownloader) {
            	auto &pDownloader = *itDownloader;

                if (it != waitTimes.end()) {
                    auto time = *it;
                    ++it;
                    
                    bool broken = false;
                    while(time > 0) {
                    	--time;
                    	CHRONO_NS::milliseconds w(1000);
                    	sleep_for(w);
                    	auto spDownloader = pDownloader.lock();
                    	if (!spDownloader) {
                    		broken = true;
                    		break;
                    	}
                    }
                    if (broken) {
                    	break;
                    }
                }
                
                auto spDownloader = pDownloader.lock();
                if (spDownloader) {
                    spDownloader->start();
                } else {
                    // we are no longer alive...
                    break;
                }
            }
        });
        
        thread.detach();
    }
    
#ifdef DEBUG
    static int counter = 0;
#endif
    
    method PlaybackData(const vector<string> &urls, const Format format, const IDownloaderFactory &downloaderFactory) :
            IPlaybackData(format),
            downloaders(createDownloaders(urls, downloaderFactory)),
            _ranges(createRanges(urls)),
            _waitTimes(computeWaitTimes(urls))
    {
#ifdef DEBUG
        cout << "construct playbackdata: " << ++counter << "\n";
#endif
        
        auto rangeIt = _ranges.begin();
        for (auto itDownloader = downloaders.begin() ; itDownloader != downloaders.end() ; ++itDownloader) {
            auto &downloader = *itDownloader;

            int begin = rangeIt->first;
            int end = rangeIt->second;
            
            ++rangeIt;
            if (rangeIt != _ranges.end()) {
                downloader->setTotalLength(rangeIt->first - begin,true);
            } else {
                downloader->setTotalLength(end - begin, true);
            }
        }
        
        if (downloaders.size() > 0) {
            (*downloaders.begin())->start();
        }
                
        startPrefetchingAtIndex(0);
        
        vector<weak_ptr<IDownloader>> weakDownloaders;
        weakDownloaders.reserve(downloaders.size());
        transform(downloaders.begin(), downloaders.end(), back_inserter(weakDownloaders), [](const shared_ptr<IDownloader> &rhs){
            return weak_ptr<IDownloader>(rhs);
        });
        
        // at least one download shall be alive
        Base::Thread thread([weakDownloaders]{
            
            // must start after delay whether we are there or not...
            
            const int kBeat = 2;
            
            for (;;) {
                
                bool br = false;
                for (auto itDownloader = weakDownloaders.begin() ; itDownloader != weakDownloaders.end() ; ++itDownloader) {
                    auto &pDownloader = *itDownloader;

                    auto spDownloader = pDownloader.lock();
                    if (!spDownloader) {
                        br = true;
                        break;
                    }
                    
                    // start the first non-running download
                    if (!spDownloader->finished()) {
                        spDownloader->start();
                        break;
                    }
                }
                if (br) {
                    break;
                }
                
                CHRONO_NS::milliseconds w(kBeat*1000);
                sleep_for(w);
            }
        });
        thread.detach();
    }
    
    method ~PlaybackData()
    {
#ifdef DEBUG
        cout << "destruct playbackdata: " << --counter << "\n";
#endif
    }
    
    // very important: offsetAvailable(), accessChunk(), and seek() must be in sync, otherwise there are crashes
    std::pair<decltype(PlaybackData::downloaders)::const_iterator,int> method findPart(int globalOffset) const
    {
        if (downloaders.size() == 1) {
            return std::make_pair(downloaders.begin(), globalOffset);
        }
        
        // start of current chunk
        for (auto it = _ranges.begin() ; it != _ranges.end() ; ++it) {
            auto &range = *it;
            if (globalOffset >= range.first && globalOffset < range.second) {
                return std::make_pair(downloaders.begin() + (it - _ranges.begin()), globalOffset - range.first);
            }
        }
        return std::make_pair(downloaders.end(), 0);
        
        /*int start = 0;
        for (auto itDownloader = downloaders.begin() ; itDownloader != downloaders.end() ; ++itDownloader) {
        	auto &downloader = *itDownloader;
        	
            int downloaderTotal = downloader->totalLength();
            
            if (downloaderTotal == 0) {
                return std::make_pair(itDownloader, globalOffset-start);
            } else {
                if (globalOffset >= start && globalOffset < (start + downloaderTotal)) {
                    return std::make_pair(itDownloader, globalOffset-start);
                }
                start += downloaderTotal;
            }
        }
        
        return std::make_pair(downloaders.end(), 0);*/
    }
    
    int method offsetAvailable(int offset) const
    {
        auto part = findPart(offset);
        if (part.first != downloaders.end()) {
            return (*part.first)->offsetAvailable(part.second);
        }
        return 0;
    }
    
    void method waitAsync(int offset, const function<void()> &f)
    {
        if (offsetAvailable(offset) >0) {
            f();
            return;
        }
        
        auto part = findPart(offset);
        if (part.first != downloaders.end()) {
            (*part.first)->waitAsync(part.second, f);
            return;
        }
        // no success, avoid hanging
        f();
    }
    
    void method seek(int offset)
    {
        if (downloaders.size() == 1) {
            (*downloaders.begin())->seek(offset);
            return;
        }
        
        auto part = findPart(offset);
        if (part.first != downloaders.end()) {
            (*part.first)->seek(part.second);
            
            // and we have to seek in the later chunks as well, without this there would be a gap in the available data
            for (auto it = part.first+1 ; it != downloaders.end() ; ++it) {
                (*it)->seek(0);
            }
            
            startPrefetchingAtIndex((part.first - downloaders.begin())+1);
        }
    }
    
    void method accessChunk(int offset, const function<void(const char *ptr, int available)> &f)
    {
        if (offsetAvailable(offset) <= 0) {
            waitSync(offset);
        }
        
        if (downloaders.size() == 1) {
            (*downloaders.begin())->accessChunk(offset, f);
            return;
        }

        auto part = findPart(offset);
        if (part.first != downloaders.end()) {
            auto itDownloader = part.first;
            auto &downloader = *itDownloader;
            int downloaderTotal = downloader->totalLength();
            // start next if we are past the first half
            if (part.second *2 > downloaderTotal) {
                auto itNext = itDownloader+1;
                if (itNext != downloaders.end()) {
                    (*itNext)->start();
                }
            }
            
            downloader->accessChunk(part.second, f);
            return;
        }
        f(nullptr, 0);
    }
    
    const vector<pair<int,int>> & method ranges() const
    {
        // old:
        // (ranges are not final so they should only be used when not very important, e.g. seeking)
        // now ranges are final!
        return _ranges;
    }
    
    int method totalLength() const
    {
        if (downloaders.size() == 0) {
            return 0;
        }
        if (downloaders.size() == 1) {
            return downloaders.front()->totalLength();
        }
        auto lastLength = downloaders.back()->totalLength();
        auto &lastRange = _ranges.back();
        if (lastLength == 0) {
            return lastRange.second;
        }
        // range incorrectly reported by a few bytes...
        return _ranges.back().first + lastLength;
    }
    
    bool method finished() const
    {
        if (failed()) {
            return true;
        }
        
        for (auto itDownloader = downloaders.begin() ; itDownloader != downloaders.end() ; ++itDownloader) {
            auto &downloader = *itDownloader;

            if (!downloader->finished()) {
                return false;
            }
        }
        return true;
        
    }
    
    int method failed() const
    {
        int i = 0;
        for (auto itDownloader = downloaders.begin() ; itDownloader != downloaders.end() ; ++itDownloader) {
        	auto &downloader = *itDownloader;
            if (!downloader) {
                return true;
            }
            if (downloader->failed()) {
#if DEBUG
                std::cout << "failed " << i << "\n";
#endif
                return true;
            }
            i++;
        }
        return false;
    }
}
