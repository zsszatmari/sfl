//
//  SSLSocket.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/12/13.
//
//

#ifndef __G_Ear_Player__SSLSocket__
#define __G_Ear_Player__SSLSocket__

#include "Socket.h"

struct ssl_st;
struct ssl_ctx_st;
typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;

namespace Gear
{
    class SSLSocket : public Socket
    {
    public:
        SSLSocket(const string &host);
        virtual ~SSLSocket();
        virtual bool connect();
        virtual void disconnect();
        virtual void write(const string &data);
        
    private:
        void doDisconnect();
        virtual ssize_t doRead(char *buf, size_t size);
        
        SSL *_sslHandle;
        SSL_CTX *_sslContext;
    };
}

#endif /* defined(__G_Ear_Player__SSLSocket__) */
