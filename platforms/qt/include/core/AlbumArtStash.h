//
//  AlbumArtShelf.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__AlbumArtShelf__
#define __G_Ear_Player__AlbumArtShelf__

#include <string>
#include "stdplus.h"
#include "SerialExecutor.h"
#include UNORDERED_MAP_H
#include SHAREDFROMTHIS_H

namespace Gear
{
    using std::string;
    using std::vector;
    using namespace Base;
    
    class PromisedImage;
    class IFileManager;
    class ISong;
    class ImageToDeliver;
    
    class core_export AlbumArtStash final : public MEMORY_NS::enable_shared_from_this<AlbumArtStash>
    {
    public:
        AlbumArtStash(const string &imageDir, const shared_ptr<IFileManager> &fileManager);
        shared_ptr<PromisedImage> art(const string &artist, const string &album, const string &title, const string &proposedUrl, long preferredDimension);
        shared_ptr<PromisedImage> art(const shared_ptr<ISong> &song, long preferredDimension);
        
    private:
        Base::SerialExecutor backgroundQueue;
        Base::SerialExecutor googleQueue;
        Base::SerialExecutor lastFmQueue;
        
        virtual vector<char> platformImage(const string &url);

        THREAD_NS::mutex _promisesMutex;
        UNORDERED_NS::unordered_map<string, MEMORY_NS::weak_ptr<ImageToDeliver>> _promises;
        const string _imageDir;
        const shared_ptr<IFileManager> _fileManager;
    };
}

#endif /* defined(__G_Ear_Player__AlbumArtShelf__) */
