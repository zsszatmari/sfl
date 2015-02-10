//
//  AlbumArtShelf.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#include "Environment.h"
// all platforms
#define LASTFM_FETCH

#include "CoreDebug.h"
#include <sstream>
#include <queue>
#include "AlbumArtStash.h"
#include "BitmapImage.h"
#include "HttpDownloader.h"
#ifdef LASTFM_FETCH
#include "LastFmService.h"
#endif
#include "IFileManager.h"
#include "BaseUtility.h"
#include "ImageToDeliver.h"
#include "ISong.h"
#include "NamedImage.h"
#include "OfflineService.h"
#include "BackgroundExecutor.h"
#include "AtomicPtr.h"
#include "IApp.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "Logger.h"
#include CHRONO_H

//#include "FApp.h"

//#undef LASTFM_FETCH

namespace Gear
{
#define method AlbumArtStash::
    
    static const int kGoogleArtDimension = 130.0f;
#ifdef TIZEN
    static const long long kCacheSize = 50 * 1000 * 1000;  // keep images for a while, but delete if cache goes too large
#else
    static const long long kCacheSize = 100 * 1000 * 1000;  // keep images for a while, but delete if cache goes too large
#endif

    using THREAD_NS::mutex;
    using THREAD_NS::lock_guard;
    using std::function;
    
    static string safeString(const string &original)
    {
        auto str = Base::toLower(original);
        string ret(str.size(), '_');
        transform(str.begin(), str.end(), ret.begin(), [](const char c){
            if (isalnum(c)) {
                return (char)c;
            } else {
                return '_';
            }
        });
        return ret;
    }
    
    method AlbumArtStash(const string &imageDir, const shared_ptr<IFileManager> &fileManager) :
        _imageDir(imageDir),
        _fileManager(fileManager)
    {
    }
    
    static void setNoArtImageIfEmpty(const shared_ptr<ImageToDeliver> &imageToDeliver)
    {
    	if (imageToDeliver->image()) {
    		return;
    	}

    	imageToDeliver->setImage(IApp::instance()->themeManager()->current()->noArt(), ImageToDeliver::Rating::NoArt);
    }

    shared_ptr<PromisedImage> method art(const shared_ptr<ISong> &song, long preferredDimension)
    {
        if (song) {
            auto artist = song->artist();
            auto album = song->album();
            auto url = song->albumArtUrl();
            
            if (preferredDimension > 0) {
                string albumArtUrlHigh = song->stringForKey("albumArtUrlHigh");
                if (!albumArtUrlHigh.empty()) {
                    url = albumArtUrlHigh;
                }
            }
            
            if (!artist.empty() || !url.empty()) {
                return art(artist, album, song->title(), url, preferredDimension);
            } else {

                auto ret = shared_ptr<ImageToDeliver>(new ImageToDeliver(preferredDimension));
        		setNoArtImageIfEmpty(ret);
        		return ret;
            }
        }
        return shared_ptr<PromisedImage>(new PromisedImage());
    }
    
    static std::pair<string,ImageToDeliver::Rating> lastFmUrl(string artist, string album, string title, long dimension, const weak_ptr<ImageToDeliver> &weakImage)
    {
#ifndef LASTFM_FETCH
        return "";
#else
        return LastFmService::getAlbumArtUrlForArtist(artist, album, title, (int)dimension, weakImage);
#endif
    }
    
    vector<char> method platformImage(const string &url)
	{
    	return vector<char>();
	}
    
    using std::queue;
    using std::pair;
    
    typedef queue<pair<CHRONO_NS::steady_clock::time_point, weak_ptr<ImageToDeliver>>> NoArtQueueType;
    static AtomicPtr<NoArtQueueType> noArtQueue;
    
    static void startNoArtTimer()
    {
        static bool running = false;
        if (running) {
            return;
        }
        running = true;
        
        BackgroundExecutor::instance().addTask([]{
            for (;;) {
                CHRONO_NS::milliseconds w(1000);
                THREAD_NS::this_thread::sleep_for(w);
                
                auto now = CHRONO_NS::steady_clock::now();
                vector<weak_ptr<ImageToDeliver>> relevant;
                
                noArtQueue.set(function<void(NoArtQueueType &)>([&](NoArtQueueType &q){
                    relevant.clear();
                    while (!q.empty()) {
                        auto item = q.front();
                        if (item.first < now) {
                            q.pop();
                            relevant.push_back(item.second);
                        } else {
                            break;
                        }
                    }
                }));
                
                for (auto &r : relevant) {
                    auto s = r.lock();
                    if (s) {
                        setNoArtImageIfEmpty(s);
                    }
                }
            }
        });
    }

    shared_ptr<PromisedImage> method art(const string &artist, const string &album, const string &title, const string &proposedUrl, long preferredDimension)
    {
        assert(preferredDimension > 0);
        
        startNoArtTimer();
        
        
//    	return shared_ptr<PromisedImage>();
#ifdef DEBUG_DISABLE_LASTFMART
        return nullptr;
#endif
        std::stringstream str;
        str << safeString(artist) << "-" << safeString(album);
        if (album.empty() && !title.empty()) {
        	// I am not sure this is right but for now (otherwise it is way too slow):
        	//str << "-unknown";


            str << "-" << safeString(title);
        }

        const string basename(str.str());

        //Logger::stream() << "fetching album art for '" << basename << "'' from " << proposedUrl << "\n";
        
        shared_ptr<ImageToDeliver> ret;
        bool alreadyDownloading = false;
  
        long width = 0;
        long height = 0;

        {
        	lock_guard<mutex> l(_promisesMutex);
			auto it = _promises.find(basename);

			if (it != _promises.end()) {
				auto w = it->second;
				ret = w.lock();
				if (ret) {
					alreadyDownloading = true;
					auto originalRequestedDimension = ret->requestedDimension();
					if (originalRequestedDimension >= preferredDimension) {
                        setNoArtImageIfEmpty(ret);
						return ret;
					}
					width = originalRequestedDimension;
					height = originalRequestedDimension;
					ret->setRequestedDimension(preferredDimension);
				} else {
					_promises.erase(it);
				}
			}
        }
        
        bool needMinimal = false;
        
        auto imageDir = _imageDir;
        auto fileManager = _fileManager;
        
        if (!alreadyDownloading) {
            ret = shared_ptr<ImageToDeliver>(new ImageToDeliver(preferredDimension));
            
            noArtQueue.set(function<void(NoArtQueueType&)>([&](queue<pair<CHRONO_NS::steady_clock::time_point, weak_ptr<ImageToDeliver>>> &q){
                q.push(std::make_pair(CHRONO_NS::steady_clock::now() + CHRONO_NS::milliseconds(3000),ret));
            }));
        }
        
        weak_ptr<ImageToDeliver> weakImage(ret);

        {
        	lock_guard<mutex> l(_promisesMutex);
        	_promises.insert(std::make_pair(basename, weakImage));
        }
        
        auto pThis = shared_from_this();
        backgroundQueue.addTask([alreadyDownloading,imageDir,fileManager,preferredDimension,basename,width,height,weakImage,needMinimal,proposedUrl,artist,album,title,pThis]() mutable {

            bool offlineMode = OfflineService::offlineMode();
        	if (!alreadyDownloading) {
                
            	auto data = fileManager->getFileWithBasename(imageDir, basename);
            	if (data.size() == 0) {
            		data = pThis->platformImage(proposedUrl);
            	}
                if (data.size()) {
                    
                    fileManager->getImageDimensions(data, width, height);
                    if (width > 0 && height > 0) {
                        auto s = weakImage.lock();
                        if (s) {
                            s->setImage(shared_ptr<Gui::IPaintable>(new Gui::BitmapImage(data, basename)), ImageToDeliver::Rating::FetchedFromCache);
                        } else {
                            return;
                        }
                    }
                } else {
                    needMinimal = true;
                }
                if (!offlineMode) {
                    fileManager->removeFilesIfExceedsCache(imageDir, kCacheSize);
                }
            }
            
            if (offlineMode) {
                auto s = weakImage.lock();
                if (s) {
                    setNoArtImageIfEmpty(s);
                }
                return;
            }
            
            auto downloadUrl = [imageDir, weakImage, fileManager, basename,title](const string &url, ImageToDeliver::Rating rating){

                auto image = weakImage.lock();
                if (!image) {
                    return false;
                }
                auto downloader = HttpDownloader::create(HttpDownloader::upgradeToHttps(url));
                downloader->waitUntilFinished();
                
                if (!downloader->failed()) {
                    vector<char> data = *downloader;
                    
                    string filename = basename;
                    if (data.size() >= 2 && ((data.at(0) & 0xFF) == 0xFF) && ((data.at(1) & 0xFF) == 0xD8)) {
                        filename.append(".jpg");
                    } else if (data.size() >= 4 && ((data.at(0) & 0xFF) == 0x89) && ((data.at(1) & 0xFF) == 0x50) && ((data.at(2) & 0xFF) == 0x4E) && ((data.at(3) & 0xFF) == 0x47)) {
                        filename.append(".png");
                    } else {
                        // possibly a 404 error html
                        return false;
                    }
                    
                    {
                        auto image = weakImage.lock();
                        if (image) {
                            //std::cout << "set " << basename << " url: " << url << " gotsize: " << data.size() << std::endl;
                            image->setImage(shared_ptr<Gui::IPaintable>(new Gui::BitmapImage(data, basename)), rating);
                            //cout << "set" << endl;
                        }
                    }
                    
                    
                    fileManager->deleteFileWithBasename(imageDir, basename);
                    fileManager->putFile(imageDir, filename, data);
                    return true;
                }
                return false;
            };
            
            bool resizableGoogleRequest = (proposedUrl.find(".googleusercontent.com/") != string::npos);
            if ((needMinimal || resizableGoogleRequest) && !proposedUrl.empty()) {
                needMinimal = false;
                
                pThis->googleQueue.addTask([proposedUrl, downloadUrl, preferredDimension, resizableGoogleRequest] {
                    
                    if (resizableGoogleRequest) {
                        // google album art is now resizable, e.g. http://lh4.googleusercontent.com/P9QOdpv_FBJOXhxD48u3CGz-ayymjx28fMYSaUtMdWyKcs-APX3EPRLViB?http://lh4.googleusercontent.com/P9QOdpv_FBJOXhxD48u3CGz-ayymjx28fMYSaUtMdWyKcs-APX3EPRLViBE0D9n3eMNcIEkMXw=s800-e100
                        std::stringstream ss;
                        ss << proposedUrl << "=s" << preferredDimension;
                        downloadUrl(ss.str(), ImageToDeliver::Rating::Provided);
                    } else {
                        downloadUrl(proposedUrl, ImageToDeliver::Rating::Provided);
                    }
                });
                if (resizableGoogleRequest) {
                    return;
                }
                
            } else {
            	auto sImage = weakImage.lock();
            	if (sImage) {
					setNoArtImageIfEmpty(sImage);
				} else {
					return;
				}
            }

            if ((width < preferredDimension && height < preferredDimension) || (width == 0 || height == 0)) {

                if (true) {
                    // debug hook
                }
                
                auto getFm = [artist, album, title, downloadUrl, weakImage](long dimension){

                    auto lastUrlAndrating = lastFmUrl(artist, album, title, dimension, weakImage);
                    
                    //std::cout << "got url for " << title << ": " << lastUrl <<  std::endl;
                    
                    if (!lastUrlAndrating.first.empty()) {
                        downloadUrl(lastUrlAndrating.first, lastUrlAndrating.second);
                    }
                    //std::cout << "downloaded url for " << title << ": " << lastUrl <<  std::endl;

                    auto sImage = weakImage.lock();
                    if (sImage) {
                    	setNoArtImageIfEmpty(sImage);
                    }
                };
                
                pThis->lastFmQueue.addTask([title,weakImage, needMinimal, artist, album, preferredDimension, downloadUrl, getFm]{

                    //std::cout << "getting url for " << title << std::endl;
                	{
                        auto image = weakImage.lock();
                        if (!image) {
                            return;
                        }
                    }
                    
                    if (needMinimal) {
                        getFm(0);
                        auto image = weakImage.lock();
                        if (!image) {
                            return;
                        }
                    }
                    
                    getFm(preferredDimension);
                });
            }
        });
        
        return ret;
    }
}
