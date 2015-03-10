//
//  SongGrouping.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/27/13.
//
//

#ifndef __G_Ear_Player__SongGrouping__
#define __G_Ear_Player__SongGrouping__

#include <string>
#include <vector>
#include "stdplus.h"
#include "Environment.h"
#include "AlbumInfo.h"
#include "SongPredicate.h"

namespace Gear
{
    using std::string;
    using std::vector;
    using std::pair;
    
    class PromisedImage;
    class ISession;
    
    class core_export SongGrouping final
    {
    public:
        SongGrouping();
        SongGrouping(const weak_ptr<ISession> &originatingSession, const string &title, const string &subtitle, const string &firstField, const string &firstValue, bool multiImage, const AlbumInfo &albumInfo, bool sortAlbumsByAlbumName);
        SongGrouping(const SongGrouping &rhs);
        SongGrouping &operator=(const SongGrouping &rhs);
        ~SongGrouping();

        explicit operator bool() const;
        bool operator==(const SongGrouping &rhs) const;
        bool operator!=(const SongGrouping &rhs) const;
        
        const string &getOrderId() const;
        const string &getId() const;
        const string &title() const;
        const string &subtitle() const;
        const string &firstField() const;
        const string &firstValue() const;
        void setSubtitle(const string &sub);
        const vector<shared_ptr<PromisedImage>> images(long dimension) const;
        bool multiImage() const;
        SongPredicate predicate() const;
        
        const vector<AlbumInfo> &albumInfos() const;
        void merge(const SongGrouping &rhs);
        shared_ptr<ISession> session() const;
        
    private:
        weak_ptr<ISession> _originatingSession;
        string _id;
        string _orderId;
        
        string _subtitle;
        string _title;
        string _firstField;
        string _firstValue;
        bool _multiImage;
        vector<AlbumInfo> _albumInfos;

        bool _sortAlbumsByAlbumName;
        
        void setId();
        
    friend void swap(SongGrouping &lhs, SongGrouping &rhs);
    };
    
    void swap(SongGrouping &lhs, SongGrouping &rhs);
}

#endif /* defined(__G_Ear_Player__SongGrouping__) */
