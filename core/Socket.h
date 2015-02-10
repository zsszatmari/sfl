//
//  Socket.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/12/13.
//
//

#ifndef __G_Ear_Player__Socket__
#define __G_Ear_Player__Socket__

#include <string>
#include <vector>
#include "Environment.h"

namespace Gear
{
    using std::string;
    using std::vector;
    
    class Socket
    {
    public:
        Socket(const string &host, int port);
        virtual ~Socket();
        virtual bool connect();
        virtual void disconnect();
        virtual void write(const string &data);
        void read(vector<char> &result);
        virtual int port() const;
        bool finished() const;
        bool failed() const;
        
    protected:
        void fail();
        int _handle;
        
    private:
        const string _host;
        bool _finished;
        bool _failed;
        int _port;
    
        void doDisconnect();
        virtual ssize_t doRead(char *buf, size_t size);
    };
}

#endif /* defined(__G_Ear_Player__Socket__) */
