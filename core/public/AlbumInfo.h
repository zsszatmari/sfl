//
//  AlbumInfo.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/27/13.
//
//

#ifndef __G_Ear_Player__AlbumInfo__
#define __G_Ear_Player__AlbumInfo__

#include <string>
#include "Environment.h"

namespace Gear
{
    using std::string;
    
    class AlbumInfo final
    {
    public:
        AlbumInfo(const string &artist, const string &album, const string &albumArtUrl);
        AlbumInfo(const AlbumInfo &info);
        ~AlbumInfo();
        
        const string artist() const;
        const string album() const;
        const string &albumArtUrl() const;
        
        bool operator<(const AlbumInfo &rhs) const;
        bool operator==(const AlbumInfo &rhs) const;
        
    private:
        string _artist;
        string _album;
        string _albumArtUrl;
    };
}

#endif /* defined(__G_Ear_Player__AlbumInfo__) */
