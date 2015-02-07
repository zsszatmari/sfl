//
//  AsyncRunner.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/18/13.
//
//

#ifndef __G_Ear__AsyncRunner__
#define __G_Ear__AsyncRunner__

#include "stdplus.h"
/*
namespace Gear
{
    using namespace boost::asio;
    
    template<class Service>
    class AsyncRunnerTemplate
    {
    public:
        static AsyncRunnerTemplate<Service> &get();
        
    private:
        AsyncRunnerTemplate();
        AsyncRunnerTemplate(const AsyncRunnerTemplate<Service> &rhs); // delete
        AsyncRunnerTemplate<Service> operator=(const AsyncRunnerTemplate<Service> &rhs); // delete
        
        Service service;
    };
    
    typedef AsyncRunnerTemplate<boost::asio::io_service> AsyncRunner;


#define method AsyncRunnerTemplate<Service>::
    
    template<class Service>
    method AsyncRunnerTemplate()
    {
        boost::thread t([&] {
            service.run();
        });
        t.detach();
    }
    
    template<class Service>
    AsyncRunnerTemplate<Service> & method get()
    {
        static AsyncRunnerTemplate<Service> runner;
        return runner;
    }
    
#undef method
}
 */

#endif /* defined(__G_Ear__AsyncRunner__) */
