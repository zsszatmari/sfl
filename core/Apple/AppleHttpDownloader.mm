//
//  HttpDownloader.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/20/13.
//
//

#include <string>
#include <sstream>
#include <iostream>
#include <Foundation/Foundation.h>
#include "AppleHttpDownloader.h"
#include "stdplus.h"
#include "StringUtility.h"
#import <SystemConfiguration/SystemConfiguration.h>
#include ATOMIC_H

// non-portable implementation

using namespace Gear;

@interface WorkerThread : NSThread {
    dispatch_semaphore_t semaphore;
    NSRunLoop *runLoop;
}

- (NSRunLoop *)runLoop;

@end

@implementation WorkerThread

using std::cout;
using std::endl;
using std::stringstream;

- (id)init
{
    self = [super init];
    if (self) {
        semaphore = dispatch_semaphore_create(0);
    }
    return self;
}

- (void)main
{
    runLoop = [NSRunLoop currentRunLoop];
    // dummy port
    [runLoop addPort:[NSMachPort port] forMode:NSDefaultRunLoopMode];
    dispatch_semaphore_signal(semaphore);
    // add a source
    try {
        [runLoop run];
    } catch(std::exception &e) {
        cout << "catch21\n";
    } catch(...) {
        cout << "catch22\n";
    }
}

- (NSRunLoop *)runLoop
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    });
    return runLoop;
}

@end

static NSRunLoop *workerRunloop()
{
    // get a runloop
    static dispatch_once_t onceToken;
    static WorkerThread *workerThread = nil;
    dispatch_once(&onceToken, ^{
        workerThread = [[WorkerThread alloc] init];
        [workerThread start];
    });
    return [workerThread runLoop];
}

@interface ConnectionDelegate : NSObject {
    MEMORY_NS::weak_ptr<AppleHttpDownloader> downloader;
}
@property(nonatomic, assign) MEMORY_NS::weak_ptr<AppleHttpDownloader> downloader;

@end

@implementation ConnectionDelegate

@synthesize downloader;

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
    auto s = self.downloader.lock();
    if (s) {
        [data retain];
        s->didReceiveData(self, (void *)data);
    }
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    auto s = self.downloader.lock();
    if (s) {
        s->didFail(self);
    }
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
    auto s = self.downloader.lock();
    if (s) {
        s->didFinish(self);
    }
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    auto s = self.downloader.lock();
    if (s) {
        if ([response isKindOfClass:[NSHTTPURLResponse class]]) {
            
            NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
            int code = [httpResponse statusCode];
            if (code == 200 || code == 206) {
            
                NSDictionary *headerFields = [httpResponse allHeaderFields];
                NSString *lengthString = [headerFields objectForKey:@"Content-Length"];
                int length = [lengthString intValue];
                s->setTotalLength(length);
            } else {
                s->didFail(self, code);
            }
        }
    }
}


@end

namespace Gear
{
    
#define method AppleHttpDownloader::
    
    shared_ptr<AppleHttpDownloader> method create(const string &url)
    {
        shared_ptr<AppleHttpDownloader> ret(new AppleHttpDownloader(url));
        map<string, string> headers;
        ret->init(url, headers);
        return ret;
    }
    
    shared_ptr<AppleHttpDownloader> method create(const string &url, const map<string,string> &extraHeaders, const string &m, const string &body)
    {
        shared_ptr<AppleHttpDownloader> ret(new AppleHttpDownloader(url, extraHeaders, m, body));
        ret->init(url, extraHeaders);
        return ret;
    }
    
    method AppleHttpDownloader(const string &url) :
    _url(url),
    _started(false),
    _method("GET")
    {
    }
    
    /*static const map<string, string> createHeadersForRangeStart(int offset)
    {
        map<string, string> headers;
        stringstream s;
        // Range: bytes=666-
        s << "bytes=";
        s << offset;
        s << "-";
        headers.insert(make_pair("Range", s.str()));
        return headers;
    }*/
    
    /*method AppleHttpDownloader(const string &url, int offset) :
    _url(url),
    _method("GET")
    {
        //init(url, createHeadersForRangeStart(offset));
    }*/
    
    method AppleHttpDownloader(const string &url, const map<string,string> &extraHeaders, const string &m, const string &body) :
        _url(url),
        _method(m),
        _body(body),
        _extraHeaders(extraHeaders)
    {
    }
    
    void method init(const string &url, const map<string,string> &extraHeaders)
    {
        @autoreleasepool {
            _tryCount = 0;
            connectionDelegate = nil;
            connection = nil;
            _startPosition = 0;
            _destructing = shared_ptr<ATOMIC_NS::atomic<bool>>(new ATOMIC_NS::atomic<bool>(0));
            semaphore = dispatch_semaphore_create(0);
            prepareDownload(url, extraHeaders);
        }
    }
    
    void method startDownload(const string &url, const map<string,string> &extraHeaders)
    {
        prepareDownload(url, extraHeaders);
        start();
    }
    
    void method prepareDownload(const string &url, const map<string,string> &extraHeaders)
    {
        _started = false;
        //_rangeStart = 0;
        _finished = 0;
        m._failed = 0;
        _available = 0;
        data.resize(0);
        
        [[NSURLCache sharedURLCache] setMemoryCapacity:0];
        NSURL *oUrl = [NSURL URLWithString:convertString(url)];
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:oUrl];
        //[request setCachePolicy:NSURLRequestReloadIgnoringLocalCacheData];
        [request setHTTPMethod:convertString(_method)];
        if (!_body.empty()) {
            NSData *data = [NSData dataWithBytes:_body.c_str() length:_body.length()];
            [request setHTTPBody:data];
        }
        for (auto &pair : extraHeaders) {
            NSString *key = convertString(pair.first);
            NSString *value = convertString(pair.second);
            
            [request setValue:value forHTTPHeaderField:key];
        }
        [(id)connectionDelegate release];
        connectionDelegate = (void *)[[ConnectionDelegate alloc] init];
        ((ConnectionDelegate *)connectionDelegate).downloader = shared_from_this();
        
        [(id)connection release];
        connection = [[NSURLConnection alloc] initWithRequest:request delegate:(ConnectionDelegate *)connectionDelegate startImmediately:NO];
        
        [(id)connection scheduleInRunLoop:workerRunloop() forMode:NSDefaultRunLoopMode];
    }
    
    void method start()
    {
        // conection == nil check because of a crash reported by David Nedde. he was using Lion
        if (!_started && connection != nil) {
            _startTime = CHRONO_NS::steady_clock::now();
            _started = true;
            @autoreleasepool {
                [(NSURLConnection *)connection start];
            }
        }
    }
    
    method ~AppleHttpDownloader()
    {
        //cout << "destruct downlodaer (begin): " << _url << "\n";
        
        *_destructing = true;
        signal();
        _queue.addTaskAndWait([]{
        });
        
        if (connectionDelegate != nil) {
            [(id)connectionDelegate release];
        }
        if (connection != nil) {
            [(id)connection cancel];
            [(id)connection release];
        }
        if (semaphore != 0) {
            dispatch_release(semaphore);
        }
        
        //cout << "destruct downlodaer (end): " << _url << "\n";
    }
    
    int method offsetAvailable(int offset) const
    {
        assert(offset >= 0);
        
        if (offset < _startPosition) {
            return 0;
        }
        offset -= _startPosition;
        int available = _available;
        
        if (available < offset) {
            return 0;
        } else {
            return available - offset;
        }
    }
    
    void method waitAsync(int offset, const function<void()> &f)
    {
        start();
        IDownloader::waitAsync(offset, f);
    }
    
    void method accessChunk(int offset, const function<void(const char *ptr, int available)> &f)
    {
        assert(offset >= 0);
        
        auto localF = f;
        waitSync(offset);
        
        shared_ptr<ATOMIC_NS::atomic<bool>> localDestructing = _destructing;
        
        auto pThis = shared_from_this();
        _queue.addTaskAndWait([pThis, localDestructing,offset,localF]{
            if (*localDestructing) {
                return;
            }
            int dataSize = (int)pThis->data.size();
            int avail = dataSize - offset + pThis->_startPosition;
            if (avail < 0) {
                // e.g. finished
                avail = 0;
            }
            localF(pThis->data.data() + offset - pThis->_startPosition, avail);
        });
    }
    
    void method didReceiveData(void *delegate, void *objcData)
    {
        auto pThis = shared_from_this();
        _queue.addTask([pThis,delegate,objcData]{
            if (delegate != pThis->connectionDelegate) {
                return;
            }
            
            NSData *oData = (NSData *)objcData;
#ifdef DEBUG
            //printf("got %ld bytes!\n", (unsigned long)[oData length]);
#endif
            
            char *bytes = (char *)[oData bytes];
            pThis->data.insert(pThis->data.end(), bytes, bytes + [oData length]);
            pThis->_available = (int)pThis->data.size();
            [oData release];
            pThis->signal();
        });
    }
    
    void method didFail(void *delegate, int code)
    {
        _tryCount++;
        NSInteger maxTries = [[NSUserDefaults standardUserDefaults] integerForKey:@"HttpRetryCount"];
        
        auto pThis = shared_from_this();
        if (_tryCount <= maxTries) {
            auto pThis = shared_from_this();
            _queue.addTask([pThis]{
                
                [(NSURLConnection *)pThis->connection cancel];
                pThis->startDownload(pThis->_url, pThis->_extraHeaders);
                //printf("restart!!\n");
            });

        } else {
        
            _queue.addTask([pThis,delegate,code]{
                if (delegate != pThis->connectionDelegate) {
                    return;
                }
                
                
                pThis->m._failed = code;
                pThis->_finished = true;
                pThis->signal();
            });
        }
    }
    
    void method didFinish(void *delegate)
    {
        auto pThis = shared_from_this();
        _queue.addTask([pThis,delegate]{
            if (delegate != pThis->connectionDelegate) {
                return;
            }
            
            pThis->_finished = true;
            pThis->signal();
        });
    }
    
    void method signal()
    {
        dispatch_semaphore_signal(semaphore);
        IAsyncData::signal();
    }
    
    bool method finished() const
    {
        return _finished;
    }
    
    int method failed() const
    {
#if DEBUG
        CHRONO_NS::steady_clock::time_point now = CHRONO_NS::steady_clock::now();
        auto elapsed = (now - _startTime);
        
        // 3000 was not enough for symphony x - iconoclast
        if (_started && !m._failed && (elapsed > CHRONO_NS::milliseconds(5000)) && _available == 0) {
            
            cout << "fail because of timeout" << endl;
            //m._failed = true;
        }
        if (m._failed ) {
            int fail = m._failed;
            if (fail != 204) {
                //cout << "failed downloading " << _url << "/" << fail << endl;
            }
        }
#endif
        
        return m._failed;
    }
    
    static dispatch_semaphore_t onlineSemaphore = 0;
    static bool available = YES;
    
    static void initSemaphore()
    {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            onlineSemaphore = dispatch_semaphore_create(1);
        });
    }
    
    bool method isOnline()
    {
        static dispatch_once_t onceToken;
        
        dispatch_once(&onceToken, ^{
            
            initSemaphore();
            
            // SCNetworkReachabilityGetFlags() may block, msut not be in main thread
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                const char *hostName = [@"google.com" cStringUsingEncoding:NSASCIIStringEncoding];
                
                while(true) {
                    SCNetworkConnectionFlags flags = 0;
                    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithName(NULL, hostName);
                    
                    bool valid = SCNetworkReachabilityGetFlags(reachability, &flags);
                    bool result = valid && ((flags & kSCNetworkReachabilityFlagsReachable) != 0) && ((flags & kSCNetworkReachabilityFlagsConnectionRequired) == 0);
                    
                    CFRelease(reachability);
                    
                    //std::cout << "availability: " << (int)valid << "/" << (int)result << "/" << flags << std::endl;
                    
                    dispatch_semaphore_wait(onlineSemaphore, DISPATCH_TIME_FOREVER);
                    BOOL update = NO;
                    if (result && !available) {
                        update = YES;
                    }
                    available = result;
                    dispatch_semaphore_signal(onlineSemaphore);
                    
                    /*
                    if (update) {
                        dispatch_async(dispatch_get_main_queue(), ^{
                            [[DatasourceManager sharedInstance] update];
                        });
                    }*/
                    sleep(1);
                }
            });
        });
        
        dispatch_semaphore_wait(onlineSemaphore, DISPATCH_TIME_FOREVER);
        bool ret = available;
        dispatch_semaphore_signal(onlineSemaphore);
        
        //std::cout << "availability returned: " << (int)ret << std::endl;
        return ret;
    }
}