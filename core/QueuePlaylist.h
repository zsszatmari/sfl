//
//  QueuePlaylist.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 28/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__QueuePlaylist__
#define __G_Ear_core__QueuePlaylist__

#include "BasicPlaylist.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
    class QueuePlaylist : public BasicPlaylist, public MEMORY_NS::enable_shared_from_this<QueuePlaylist>
    {
    public:
    	static shared_ptr<QueuePlaylist> create();
        
        virtual bool editable() const;        
        virtual bool moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before);
    
		virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        virtual void addSongEntries(const vector<SongEntry> &songs);
        virtual bool nameEditable() const;

	private:
        QueuePlaylist();
    };
}

#endif /* defined(__G_Ear_core__QueuePlaylist__) */
