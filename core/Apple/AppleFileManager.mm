//
//  AppleFileManager.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif
#include "Environment.h"
#include "AppleFileManager.h"
#include "StringUtility.h"

namespace Gear
{
#define method AppleFileManager::
    
    static NSMutableDictionary *dirEntriesCache;
    
    method AppleFileManager()
    {
        @autoreleasepool {
            dirEntriesCache = [NSMutableDictionary dictionary];
        }
    }
    
    NSMutableArray * method dirEntries(NSURL *dirUrl) const
    {
        @synchronized(dirEntriesCache) {
            NSMutableArray *contents = [dirEntriesCache objectForKey:dirUrl];
            if (contents == nil) {
                NSFileManager *fm = [NSFileManager defaultManager];
                contents = [[fm contentsOfDirectoryAtURL:dirUrl includingPropertiesForKeys:@[NSURLContentAccessDateKey, NSURLFileSizeKey] options:0 error:nil] mutableCopy];
                if (contents == nil) {
                    contents = [NSMutableArray array];
                }
                [dirEntriesCache setObject:contents forKey:dirUrl];
            }
            
            return contents;
        }
    }
    
    void method removeFilesIfExceedsCache(const string &dir, const long long maxSize)
    {
        @autoreleasepool {
            NSFileManager *fm = [NSFileManager defaultManager];
            NSMutableArray *contents = dirEntries([NSURL fileURLWithPath:convertString(dir)]);
            
            @synchronized(contents) {
                long long totalSize = 0;
                for (NSURL *item in contents) {
                    NSNumber *fileSize = nil;
                    [item getResourceValue:&fileSize forKey:NSURLFileSizeKey error:nil];
                    totalSize += [fileSize longValue];
                }
                
                if (totalSize <= maxSize) {
                    return;
                }
                
                NSArray *sortedContents = [contents sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
                            
                            NSDate *lDate = nil;
                            NSDate *rDate = nil;
                            
                            [obj1 getResourceValue:&lDate forKey:NSURLContentAccessDateKey error:nil];
                            [obj2 getResourceValue:&rDate forKey:NSURLContentAccessDateKey error:nil];
                            if (lDate == nil) {
                                if (rDate == nil) {
                                    return NSOrderedSame;
                                }
                                return NSOrderedAscending;
                            } else if (rDate == nil) {
                                return NSOrderedDescending;
                            }
                            
                            return [lDate compare:rDate];
                }];
            
                for (NSURL *item in sortedContents) {
                    NSNumber *fileSize = nil;
                    [item getResourceValue:&fileSize forKey:NSURLFileSizeKey error:nil];
                    
                    [fm removeItemAtURL:item error:nil];
                    @synchronized(contents) {
                        [contents removeObject:item];
                    }
                    totalSize -= [fileSize longValue];
                    if (totalSize <= maxSize) {
                        break;
                    }
                }
            }
        }
    }
    
    vector<char> method getFileWithBasename(const string &dir, const string &basename) const
    {
        @autoreleasepool {
            //NSFileManager *fm = [NSFileManager defaultManager];
            NSURL *dirUrl = [NSURL fileURLWithPath:convertString(dir)];
            
            NSArray *contents = dirEntries(dirUrl);
            
            NSString *base = convertString(basename);
            @synchronized(contents) {
                for (NSURL *item in contents) {
                    if ([[item lastPathComponent] hasPrefix:base]) {
                        NSData *data = [NSData dataWithContentsOfURL:item];
                        return vector<char>((char *)[data bytes], ((char *)[data bytes]) + [data length]);
                    }
                }
            }
            return vector<char>();
        }
    }
    
    void method deleteFileWithBasename(const string &dir, const string &basename)
    {
        @autoreleasepool {
            NSFileManager *fm = [NSFileManager defaultManager];
            
            NSMutableArray *contents = dirEntries([NSURL fileURLWithPath:convertString(dir)]);
            
            NSString *base = convertString(basename);
            NSMutableArray *toDelete = [NSMutableArray array];
            @synchronized(contents) {
                for (NSURL *item in contents) {
                    if ([[item lastPathComponent] hasPrefix:base]) {
                        [toDelete addObject:item];
                    }
                }
                for (NSURL *item in toDelete) {
                    [fm removeItemAtURL:item error:nil];
                    [contents removeObject:item];
                }
            }
        }
    }

    void method deleteFile(const string &file)
    {
        @autoreleasepool {
            [[NSFileManager defaultManager] removeItemAtPath:convertString(file) error:nil];
        }
    }
    
    void method putFile(const string &dir, const string &filename, const vector<char> &data)
    {
        @autoreleasepool {
            NSData *d = [NSData dataWithBytesNoCopy:(void *)&data.front() length:data.size() freeWhenDone:NO];
            NSURL *dirUrl = [NSURL fileURLWithPath:convertString(dir) isDirectory:YES];
            NSURL *url = [dirUrl URLByAppendingPathComponent:convertString(filename)];
            NSError *err = nil;
            [d writeToURL:url options:NSAtomicWrite error:&err];
            
            NSMutableArray *contents = dirEntries(dirUrl);
            @synchronized(contents) {
                [contents addObject:url];
            }
        }
    }
    
    void method getImageDimensions(const vector<char> &data, long &width, long &height)
    {
        @autoreleasepool {
            NSData *d = [NSData dataWithBytesNoCopy:(void *)&data.front() length:data.size() freeWhenDone:NO];

    #if TARGET_OS_IPHONE
            UIImage *image = [[UIImage alloc] initWithData:d];
            CGSize size = [image size];
    #else
            NSImage *image = [[NSImage alloc] initWithData:d];
            NSSize size = [image size];
    #endif
            
            width = size.width;
            height = size.height;
        }
    }
    
    vector<string> method listFiles(const string &prefix)
    {
        size_t lastSlash = prefix.find_last_of('/');
        string dir;
        string filenamePrefix;
        if (lastSlash == string::npos) {
            dir = prefix;
        } else {
            dir = prefix.substr(0, lastSlash);
            filenamePrefix = prefix.substr(lastSlash +1);
        }
        
        @autoreleasepool {
            //NSDirectoryEnumerator *enumerator = [[NSFileManager defaultManager] enumeratorAtPath:convertString(dir)];
            NSArray *enumerator = [[[NSFileManager defaultManager] contentsOfDirectoryAtURL:[NSURL fileURLWithPath:convertString(dir)] includingPropertiesForKeys:@[NSURLContentAccessDateKey, NSURLFileSizeKey] options:0 error:nil] mutableCopy];
            NSString *pref = convertString(filenamePrefix);
            vector<string> ret;
            for (NSURL *url in enumerator) {
                NSNumber *size = nil;
                [url getResourceValue:&size forKey:NSURLFileSizeKey error:nil];
                if ([size intValue] == 0) {
                    continue;
                }
                
                NSString *filename = [url lastPathComponent];
                if ([filename hasPrefix:pref]) {
                    ret.push_back(convertString([filename substringFromIndex:[pref length]]));
                }
            }
            
            return ret;
        }
    }
}
