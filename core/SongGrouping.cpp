//
//  SongGrouping.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/27/13.
//
//

#include "BaseUtility.h"
#include "SongGrouping.h"
#include "MusicSort.h"
#include "AlbumInfo.h"
#include "IApp.h"
#include "AlbumArtStash.h"
#include "IPreferences.h"

//#define DEBUG_COUNTER

namespace Gear
{
#define method SongGrouping::
    
    method SongGrouping()
    {
    }
    
    static inline vector<AlbumInfo> albumVector(const AlbumInfo &info)
    {
        AlbumInfo s[] = {info};
        return Base::init<vector<AlbumInfo>>(s);
    }
    
#ifdef DEBUG_COUNTER
    static int count = 0;
#endif
    
    method SongGrouping(const string &title, const string &subtitle, const string &firstField, const string &firstValue, bool multiImage, const AlbumInfo &albumInfo, const function<shared_ptr<IPlaylist>(const SongGrouping &)> &createPlaylist) :
        _title(title),
        _subtitle(subtitle),
        _firstField(firstField),
        _firstValue(firstValue),
        _multiImage(multiImage),
        _albumInfos(albumVector(albumInfo)),
        _createPlaylist(createPlaylist)
    {
#ifdef DEBUG_COUNTER
        ++count;
        std::cout << "grouping inc: " << count << " (" << (int)(this) << ")" << std::endl;
#endif
        assert(_createPlaylist);
        setId();
    }
    
    method SongGrouping(const SongGrouping &rhs) :
        //_originatingSession(rhs._originatingSession),
        _id(rhs._id),
        _subtitle(rhs._subtitle),
        _title(rhs._title),
        _firstField(rhs._firstField),
        _firstValue(rhs._firstValue),
        _multiImage(rhs._multiImage),
        _albumInfos(rhs._albumInfos),
        _createPlaylist(rhs._createPlaylist)
    {
#ifdef DEBUG_COUNTER
        ++count;
        std::cout << "grouping cop: " << count << " (" << (int)(this) << ")" << std::endl;
#endif
    }
    
    method ~SongGrouping()
    {
#ifdef DEBUG_COUNTER
        --count;
        std::cout << "grouping dec: " << count << " (" << (int)(this) << ")" << std::endl;
#endif
    }
    
    SongGrouping & method operator=(const SongGrouping &rhs)
    {
    	//_originatingSession = rhs._originatingSession.lock();
    	_id = rhs._id;
    	_subtitle = rhs._subtitle;
    	_title = rhs._title;
    	_firstField = rhs._firstField;
    	_firstValue = rhs._firstValue;
    	_multiImage = rhs._multiImage;
    	_albumInfos = rhs._albumInfos;
        _createPlaylist = rhs._createPlaylist;

    	return *this;
    }
    
    void swap(SongGrouping &lhs, SongGrouping &rhs)
    {
        /*if (&lhs == &rhs) {
            return;
        }*/
        
        //swap(lhs._originatingSession, rhs._originatingSession);
        swap(lhs._id, rhs._id);
        swap(lhs._subtitle, rhs._subtitle);
        swap(lhs._title, rhs._title);
        swap(lhs._firstField, rhs._firstField);
        swap(lhs._firstValue, rhs._firstValue);
        lhs._multiImage = lhs._multiImage ^ rhs._multiImage;
        rhs._multiImage = lhs._multiImage ^ rhs._multiImage;
        lhs._multiImage = lhs._multiImage ^ rhs._multiImage;
        swap(lhs._albumInfos, rhs._albumInfos);
    }

    /*shared_ptr<ISession> method session() const
	{
    	return _originatingSession.lock();
	}*/

    method operator bool() const
    {
        return !_id.empty();
    }
    
    const string & method title() const
    {
        return _title;
    }
    
    const string & method subtitle() const
    {
        return _subtitle;
    }
    
    void method setSubtitle(const string &sub)
    {
        _subtitle = sub;
        setId();
    }
    
    void method setId()
    {
        char buffer[255];
        string sub = _subtitle;
        if (sub.empty()) {
            sub = "zzzzzzz";
        }
        string title = _title;
        if (title.empty()) {
            title = "zzzzzzz";
        }
        
#ifdef _WIN32
#define snprintf _snprintf
#endif
        //snprintf(buffer, sizeof(buffer), "%s-%s", title.c_str(), Base::toLower(MusicSort::strip(sub)).c_str() );
        snprintf(buffer, sizeof(buffer), "%s-%s", title.c_str(), MusicSort::strip(sub).c_str() );

        // merge mistyped artist names. but this is a bad idea now, because when user clicks, the others are still not there
        //snprintf(buffer, sizeof(buffer), "%s-%s", Base::toLower(title).c_str(), Base::toLower(MusicSort::strip(sub)).c_str() );
        
        _id = buffer;
    }
    
    void method merge(const SongGrouping &rhs)
    {
        for (auto &albumInfo : rhs._albumInfos) {
            auto insertPosition = std::lower_bound(_albumInfos.begin(), _albumInfos.end(), albumInfo);
            
            // avoid duplicates
            if (insertPosition == _albumInfos.end() || albumInfo < *insertPosition) {
                _albumInfos.insert(insertPosition, albumInfo);
            }
        }
        
        /*if (rhs._originatingSession.lock() != _originatingSession.lock()) {
        	_originatingSession = weak_ptr<ISession>();
        }*/
    }

    const vector<AlbumInfo> & method albumInfos() const
    {
        return _albumInfos;
    }
    
    const vector<shared_ptr<PromisedImage>> method images(long dimension) const
    {
        vector<shared_ptr<PromisedImage>> ret;
        auto stash = IApp::instance()->albumArtStash();
        transform(_albumInfos.begin(), _albumInfos.end(), back_inserter(ret), [stash, dimension](const AlbumInfo &rhs){
            
            return stash->art(rhs.artist(), rhs.album(), "", rhs.albumArtUrl(), dimension);
        });
        return ret;
    }
    
    const string & method firstField() const
    {
        return _firstField;
    }
    
    const string & method firstValue() const
    {
        return _firstValue;
    }
    
    bool method multiImage() const
    {
        return _multiImage;
    }
    
    bool method operator==(const SongGrouping &rhs) const
    {
        return _id == rhs._id;
    }
    
    bool method operator!=(const SongGrouping &rhs) const
	{
		return !operator==(rhs);
	}
    
    SongPredicate method predicate() const
    {
        return SongPredicate(firstField(), firstValue(), SongPredicate::Matches());
    }

    shared_ptr<IPlaylist> method playlist() const
    {
        return _createPlaylist(*this);
    }


#undef method

}

