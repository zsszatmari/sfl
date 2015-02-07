//
//  DecoderAbstract.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 11/19/12.
//
//

#import "DecoderAbstract.h"

const int kDecoderNoError = 0;
const int kDecoderNeedMore = 1;
const int kDecoderEnd = 2;
const int kDecoderUnknownError = 3;


@implementation DecoderAbstract

- (int)readInto:(unsigned char *)outData size:(size_t)outSize producedBytes:(size_t *)producedBytes
{
    NSAssert(NO, @"abstract method");
    return 0;
}

- (void)feedFrom:(unsigned const char *)inData size:(size_t)inSize
{
    NSAssert(NO, @"abstract method");
}

- (long)seekOffset:(NSTimeInterval)time firstGuess:(size_t)guess;
{
    NSAssert(NO, @"abstract method");
    return 0;
}

@end
