//
//  SSLSocket.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/12/13.
//
//

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "SSLSocket.h"
#include "Environment.h"

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "lib\\win\\libeay32.lib")
#pragma comment(lib, "lib\\win\\ssleay32.lib")
#endif

namespace Gear
{
    class SSLStatic
    {
    public:
        SSLStatic()
        {
#ifdef _WIN32
            WSADATA wsaData;
            int err;

            /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
            WORD wVersionRequested = MAKEWORD(2, 2);

            err = WSAStartup(wVersionRequested, &wsaData);
#endif


            // Register the error strings for libcrypto & libssl
            SSL_load_error_strings ();
            // Register the available ciphers and digests
            SSL_library_init ();


        }
    };
    static SSLStatic sslStatic;
    
    
#define method SSLSocket::
    
    method SSLSocket(const string &host) :
        Socket(host, 443),
        _sslContext(0),
        _sslHandle(0)
    {
    }
    
    method ~SSLSocket()
    {
        doDisconnect();
    }
    
    bool method connect()
    {
        bool success = Socket::connect();
        if (!success) {
            return false;
        }
        
        // New context saying we are a client, and using SSL 2 or 3
        _sslContext = SSL_CTX_new (SSLv23_client_method ());
        if (_sslContext == NULL) {
            ERR_print_errors_fp (stderr);
            fail();
            return false;
        }
        
        // Create an SSL struct for the connection
        _sslHandle = SSL_new(_sslContext);
        if (_sslHandle == NULL) {
            
            ERR_print_errors_fp (stderr);
            fail();
            return false;
        }
        
        // Connect the SSL struct to our connection
        if (!SSL_set_fd (_sslHandle, _handle)) {
            ERR_print_errors_fp (stderr);
            fail();
            return false;
        }
        
        // Initiate SSL handshake
		int ret = SSL_connect(_sslHandle);
        if (ret != 1) {
            ERR_print_errors_fp (stdout); fflush(NULL);
            int e = SSL_get_error(_sslHandle, ret);
            // err: 5 system error
#ifdef DEBUG
#ifdef _WIN32
            int syserr = GetLastError();
#endif
#endif
            fail();
            return false;
        }
        return true;
    }
    
    void method disconnect()
    {
        doDisconnect();
    }
    
    void method doDisconnect()
    {
        Socket::disconnect();

        if (_sslHandle)
        {
            SSL_shutdown (_sslHandle);
            SSL_free (_sslHandle);
        }
        if (_sslContext) {
            SSL_CTX_free (_sslContext);
        }
    }

    void method write(const string &data)
    {
        if (!finished()) {
            ssize_t written = SSL_write (_sslHandle, data.data(), (int)data.length());
            if (written != data.length()) {
                fail();
            }
        }
    }
    
    ssize_t method doRead(char *buf, size_t size)
    {
        return ::SSL_read(_sslHandle, buf, size);
    }
}
