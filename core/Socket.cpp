//
//  Socket.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/12/13.
//
//

#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#endif
#include <sys/types.h>
#include "Socket.h"

namespace Gear
{
#define method Socket::
    
    method Socket(const string &host, int port) :
        _host(host),
        _handle(0),
        _finished(false),
        _failed(false),
        _port(port)
    {
    }
    
    method ~Socket()
    {
        doDisconnect();
    }
    
	bool method connect()
	{
		struct addrinfo hints, *servinfo, *p;
		int rv;

		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
		hints.ai_socktype = SOCK_STREAM;

		char portstr[sizeof(int)* 2 + 1];
		sprintf(portstr, "%d", _port);
		if ((rv = getaddrinfo(_host.c_str(), portstr, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			fail();
			return false;
		}

		// loop through all the results and connect to the first we can
		for (p = servinfo; p != NULL; p = p->ai_next) {
			if ((_handle = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				perror("socket");
				continue;
			}

			if (::connect(_handle, p->ai_addr, p->ai_addrlen) == -1) {
#ifdef _WIN32
				closesocket(_handle);
#else
				close(_handle);
#endif
				_handle = 0;
				perror("connect");
				continue;
			}

			break; // if we get here, we must have connected successfully
		}

		freeaddrinfo(servinfo); // all done with this structure

		if (p == NULL) {
			_handle = 0;
			// looped off the end of the list with no connection
			fprintf(stderr, "failed to connect\n");
			fail();
			return false;
		}
		return true;
	}
    
    void method fail()
    {
        _finished = true;
        _failed = true;
    }
    
    int method port() const
    {
        return _port;
    }
    
    void method disconnect()
    {
        doDisconnect();
    }
    
    void method doDisconnect()
    {
        if (_handle != 0) {
#ifdef _WIN32
			closesocket(_handle);
#else
            close(_handle);
#endif
            _handle = 0;
        }
    }
    
    void method write(const string &data)
    {
        ssize_t written = ::send(_handle, data.c_str(), data.length(),0);
        if (written != data.size()) {
            fail();
        }
    }
    
    bool method finished() const
    {
        return _finished;
    }
    
    bool method failed() const
    {
        return _failed;
    }
    
    void method read(vector<char> &result)
    {
        // this makes little sense
        /*
        size_t size = result.capacity();
        if (size < 8192) {
            result.reserve(8192);
            size = 8192;
        }*/
        size_t size = 8192;
        result.resize(size);
        ssize_t received = doRead(result.data(), result.size());
        if (received < 0) {
            _finished = true;
            _failed = true;
            result.resize(0);
        } else if (received == 0) {
            result.resize(0);
            _finished = true;
        } else {
            result.resize(received);
        }
    }
        
    ssize_t method doRead(char *buf, size_t size)
    {
        return ::recv(_handle, buf, size, 0);
    }
}
