//
//  GooglePlayRadio.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/4/13.
//
//

#ifndef __G_Ear__GooglePlayRadio__
#define __G_Ear__GooglePlayRadio__

#include "IPlaylist.h"
#include "ISongArray.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
    class GooglePlaySession;
    class SortedSongArray;
    
    class GooglePlayRadio final : public IPlaylist, public MEMORY_NS::enable_shared_from_this<GooglePlayRadio>
    {
    public:
        static shared_ptr<GooglePlayRadio> create(const string &name, const string &radioId, const weak_ptr<GooglePlaySession> &session);
        
        virtual const string name() const;
        virtual void setName(const string &name);
        virtual const shared_ptr<ISongArray> songArray();
        virtual bool removable() const;
        virtual void remove();
        const string &radioId() const;
        virtual const string playlistId() const;
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        virtual shared_ptr<PromisedImage> image(int preferredSize) const;
        virtual vector<shared_ptr<IPlaylistIntent>>playlistIntents();
        virtual bool saveForOfflinePossible() const;
        virtual bool keepSongOrder() const;
        virtual bool editable() const;
        void setEditable(bool editable);
        virtual void refresh() override;
        
    private:
        GooglePlayRadio(const string &name, const string &radioId, const weak_ptr<GooglePlaySession> &session);
        
        GooglePlayRadio(const GooglePlayRadio &rhs); // delete
        GooglePlayRadio &operator=(const GooglePlayRadio &rhs); // delete
        
        string _name;
        const string _radioId;
        weak_ptr<SortedSongArray> _songArray;
        bool _editable;
        //shared_ptr<ISongArray> _retainedArray;
        const weak_ptr<GooglePlaySession> _radioSession;
    };
}

#endif /* defined(__G_Ear__GooglePlayRadio__) */
