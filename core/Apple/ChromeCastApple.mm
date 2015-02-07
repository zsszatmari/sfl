#import <Foundation/Foundation.h>
#include "ChromeCastApple.h"
#include "CastDevice.h"
#include "StringUtility.h"
#include "ChromeCast.h"

@interface ChromeCastAppleHelper : NSObject<NSNetServiceBrowserDelegate,NSNetServiceDelegate>

@property(nonatomic,assign) function<void(const shared_ptr<Cast::CastDevice> &)> callback;

@end

@implementation ChromeCastAppleHelper {
	NSMutableArray *candidates;
	THREAD_NS::mutex m;
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser
           didFindService:(NSNetService *)service
               moreComing:(BOOL)moreServicesComing
{
	THREAD_NS::lock_guard<THREAD_NS::mutex> l(m);
    [service setDelegate:self];
    if (candidates == nil) {
    	candidates = [[NSMutableArray alloc] init];
    }
    [candidates addObject:service];

    [service resolveWithTimeout:30.0f];
}

- (void)netService:(NSNetService *)sender
     didNotResolve:(NSDictionary *)errorDict
{
	THREAD_NS::lock_guard<THREAD_NS::mutex> l(m);
	[candidates removeObject:sender];
}

- (void)netServiceDidResolveAddress:(NSNetService *)sender
{
	THREAD_NS::lock_guard<THREAD_NS::mutex> l(m);
	[candidates removeObject:sender];

	struct sockaddr_in  *socketAddress = nil;
    
    NSArray *addresses = [sender addresses];
    if ([addresses count] == 0) {
        return;
    }
 
    socketAddress = (struct sockaddr_in *)[[addresses objectAtIndex:0] bytes];
    std::string ipString(inet_ntoa(socketAddress->sin_addr));
    auto name = Gear::convertString([sender name]);

    auto cb = self.callback;
    Cast::ChromeCast::instance()->io().dispatch([name,ipString,cb]{
        cb(Cast::CastDevice::create(name,ipString));
    });
}


@end

namespace Cast
{
#define method ChromeCastApple::

	void method probeAsync(const function<void(const shared_ptr<CastDevice> &)> &aCallback)
	{
        auto callback = aCallback;
        dispatch_async(dispatch_get_main_queue(),^{
            static NSNetServiceBrowser *browser = [[NSNetServiceBrowser alloc] init];
            static ChromeCastAppleHelper *helper = [[ChromeCastAppleHelper alloc] init];
            helper.callback = callback;
            [browser setDelegate:helper];
            //[browser stop];
            [browser searchForServicesOfType:@"_googlecast._tcp" inDomain:@"local."];
        });
	}
}
