//
//  EQController.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/24/13.
//
//

#import <Foundation/Foundation.h>

@interface EQController : NSObject

@property (nonatomic, assign) BOOL windowVisible;
@property (unsafe_unretained) IBOutlet NSPanel *window;

@end
