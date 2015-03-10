//
//  PlaylistCell.m
//  G-Ear iOS
//
//  Created by Gál László on 7/25/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "PlaylistCell.h"
#include "StringUtility.h"
#include "TextAttributes.h"
#include "Writer.h"
#include "IApp.h"
#include "ISession.h"
#include "IPlayer.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "SessionManager.h"
#import "SongCell.h"

using namespace Gear;

@implementation PlaylistCell {
    SignalConnection _playlistConnection;
}

- (void)setPlaylist:(shared_ptr<IPlaylist>)playlist
{
    if (playlist == nil) {
        return;
    }
    
    [self.nameLabel setText:convertString(playlist->name())];
    Gui::Writer::apply(self.nameAttributes, self.nameLabel);
    
	shared_ptr<Gui::IPaintable> simage = IApp::instance()->sessionManager()->sessionIcon(playlist);
	if (simage) {
		//CGRect _rect = self.nameLabel.frame;
		_sourceImage.image = simage;
		//_rect.size.width -= SourceWidth;
		//_nameLabel.frame = _rect;
	} else {
        _sourceImage.image = nil;
    }
    
    _playlistConnection = IApp::instance()->player()->playlistCurrentlyPlayingConnector().connect([self,playlist](const shared_ptr<IPlaylist> &current){
        bool selected = false;
        if (playlist == current) {
            selected = true;
        }
        auto theme = IApp::instance()->themeManager()->current();
        auto attrib = theme->listTitleText(selected);
        Gui::Writer::apply(attrib, _nameLabel);
    });
}

- (void)setSeparatorThickness:(int)separatorThickness
{
    CGRect sepBounds = self.separator.bounds;
    sepBounds.size.height = separatorThickness;
    self.separator.bounds = sepBounds;
}

@end
