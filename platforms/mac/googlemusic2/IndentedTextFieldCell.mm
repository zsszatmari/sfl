//
//  IndentedTextFieldCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 10/1/12.
//
//

#import "IndentedTextFieldCell.h"
#import "AppDelegate.h"
#include "IApp.h"
#include "Painter.h"
#include "ISession.h"
#include "IPlayer.h"
#include "SessionManager.h"
#include "IPlaylist.h"
#import "PlaylistManager.h"
#import "CocoaThemeManager.h"

using namespace Gear;
@interface ClickableTextField : NSTextField

@property (nonatomic, assign) shared_ptr<IPlaylist> playlist;

@end
@implementation IndentedTextFieldCell

static BOOL editingWhenCapable = NO;

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    BOOL sourceDrawn = NO;
	if ([controlView isKindOfClass: [ClickableTextField class]]) {
		shared_ptr<Gui::IPaintable> sourceImage = Gear::IApp::instance()->sessionManager()->sessionIcon([(ClickableTextField *) controlView playlist]);
		if (sourceImage) {
            sourceDrawn = YES;
			Gui::Painter::paint(*sourceImage, NSRectToCGRect(NSMakeRect(kPlaylistIconPositionLeft -2, (cellFrame.size.height - 14) / 2, 14, 14)));
		}
	}
//    const CGFloat kIndent = 10.0f;
    const CGFloat kIndent = (sourceDrawn ? kPlaylistPositionLeftIfIconPresent : kPlaylistPositionLeft) -1;
    cellFrame.origin.x = cellFrame.origin.x + kIndent;
    cellFrame.size.width = MAX(cellFrame.size.width - kIndent,0);
    
    BOOL isBold = [[[self font] fontName] hasSuffix:@"Bold"];
    cellFrame.origin.y -= 1 - (!isBold ? 3 : 0);
    
    
    static NSInteger customOffset = [[NSUserDefaults standardUserDefaults] integerForKey:@"PlaylistOffset"];
    cellFrame.origin.y += customOffset;
    
    if (self.selected) {
        [self setTextColor:[[CocoaThemeManager sharedManager] playlistsTextSelectedColor]];
        //[self setTextColor:[NSColor redColor]];
    } else {
        [self setTextColor:[[CocoaThemeManager sharedManager] playlistsTextColor]];
    }
    [[self attributedStringValue] drawInRect:cellFrame];
    /*NSMutableAttributedString *string = [[self attributedStringValue] mutableCopy];
    [string addAttributes:@{NSForegroundColorAttributeName:[self textColor]} range:NSMakeRange(0,[string length])];
    [string drawInRect:cellFrame];*/
    
    //[super drawInteriorWithFrame:cellFrame inView:controlView];
}

- (BOOL)isEditable
{
    if (lionOrBetter()) {
        return editCapable && editingWhenCapable;
    } else {
        return [super isEditable];
    }
}

- (BOOL)editCapable
{
    return editCapable;
}

- (void)setEditCapable:(BOOL)value
{
    if (lionOrBetter()) {
        editCapable = value;
    } else {
        [self setEditable:value];
    }
}

+ (void)enableEditingWhenCapable
{
    editingWhenCapable = YES;
}

+ (void)disableEditingWhenCapable
{
    editingWhenCapable = NO;
}

@end
