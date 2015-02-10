//
//  AlbumInfo.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/27/13.
//
//

#include <iostream>
#include "AlbumInfo.h"
#include "BaseUtility.h"
#include "MusicSort.h"

//#define DEBUG_COUNT_ALBUMINFO

namespace Gear
{
#define method AlbumInfo::
    
#ifdef DEBUG_COUNT_ALBUMINFO
    static int count = 0;
#endif
    
    method AlbumInfo(const string &artist, const string &album, const string &albumArtUrl) :
        _artist(artist),
        _album(album),
        _albumArtUrl(albumArtUrl)
    {
#ifdef DEBUG_COUNT_ALBUMINFO
        count++;
        if (count % 1000 == 0)
            std::cout << "albuminfo inc: " << count << " (" << (int)(this) << ")" << std::endl;
#endif
    }
    
    method AlbumInfo(const AlbumInfo &rhs) :
        _artist(rhs._artist),
        _album(rhs._album),
        _albumArtUrl(rhs._albumArtUrl)
    {
#ifdef DEBUG_COUNT_ALBUMINFO
        count++;
        if (count % 1000 == 0)
            std::cout << "albuminfo cop: " << count << " (" << (int)(this) << ")" << std::endl;
#endif
    }
    
    method ~AlbumInfo()
    {
#ifdef DEBUG_COUNT_ALBUMINFO
        count--;
        //std::cout << "albuminfo dec: " << count << " (" << (int)(this) << ")" << std::endl;
#endif
    }
    
    const string method artist() const
    {
        return _artist;
    }
    
    const string method album() const
    {
        return _album;
    }
    
    const string & method albumArtUrl() const
    {
        return _albumArtUrl;
    }

    bool method operator<(const AlbumInfo &rhs) const
    {
        if (_artist < rhs._artist) {
            return true;
        }
        if (_artist > rhs._artist) {
            return false;
        }
        if (_album < rhs._album) {
            return true;
        }
        // album art url can differ even for the same album! don't use in the equality check
        return false;
        /*if (_album > rhs._album) {
            return false;
        }
        if (_albumArtUrl < rhs._albumArtUrl) {
            return true;
        }
        return false;*/
    }
    
    bool method operator==(const AlbumInfo &rhs) const
    {            
        bool ret = _artist == rhs._artist && _album == rhs._album; // && _albumArtUrl == rhs._albumArtUrl;
        return ret;
    }
}