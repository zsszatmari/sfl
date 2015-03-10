//
//  DecoderAbstract.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 11/19/12.
//
//

#import <Foundation/Foundation.h>

extern const int kDecoderNoError;
extern const int kDecoderNeedMore;
extern const int kDecoderEnd;
extern const int kDecoderUnknownError;

@interface DecoderAbstract : NSObject

- (int)readInto:(unsigned char *)outData size:(size_t)outSize producedBytes:(size_t *)producedBytes;
- (void)feedFrom:(unsigned const char *)inData size:(size_t)inSize;
- (long)seekOffset:(NSTimeInterval)time firstGuess:(size_t)guess;

@end
