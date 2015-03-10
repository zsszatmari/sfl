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
#include <sstream>
#include "GenericHttpDownloader.h"
#include "BackgroundExecutor.h"
#include "stdplus.h"
#include "IApp.h"
#include "IPreferences.h"
#include "SSLSocket.h"
#include "BaseUtility.h"

//#define DEBUG_HTTP
//#define DEBUG_HTTP_VERBOSE
//#define DEBUG_DISABLE_REUSE

#ifdef DEBUG
#define DEBUG_FAILING
#endif

using namespace Gear;

namespace Gear
{
    using std::cout;
    using std::endl;
    using THREAD_NS::mutex;
    using THREAD_NS::lock_guard;
    using THREAD_NS::unique_lock;
    
#define method GenericHttpDownloader::
    
    shared_ptr<GenericHttpDownloader> method create(const string &url)
    {
        shared_ptr<GenericHttpDownloader> ret(new GenericHttpDownloader(url));
        return ret;
    }
    
    shared_ptr<GenericHttpDownloader> method create(const string &url, const map<string,string> &extraHeaders, const string &m , const string &body)
    {
        shared_ptr<GenericHttpDownloader> ret(new GenericHttpDownloader(url, extraHeaders, m, body));
                                       
        return ret;
    }
    
    static const map<string, string> createHeadersForRangeStart(int offset)
    {
        map<string, string> headers;
        std::stringstream s;
        // Range: bytes=666-
        s << "bytes=";
        s << offset;
        s << "-";
        headers.insert(make_pair("Range", s.str()));
        return headers;
    }
    
    
    method GenericHttpDownloader(const string &url) :
    _url(url),
    _started(false),
    _method("GET")
    {
        map<string, string> headers;
        init(url, headers);
    }
    
    
    method GenericHttpDownloader(const string &url, int offset) :
        _url(url),
        _method("GET")
    {
        init(url, createHeadersForRangeStart(offset));
    }
    
    method GenericHttpDownloader(const string &url, const map<string,string> &extraHeaders, const string &m, const string &body) :
        _url(url),
        _method(m),
        _body(body)
    {
        init(url, extraHeaders);
    }
    
    void method init(const string &url, const map<string,string> &extraHeaders)
    {
        prepareDownload(url, extraHeaders);
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
        _startPosition = 0;
        _finished = 0;
        m._failed = 0;
        _payload.reset();
        _headers = extraHeaders;
    }
    
    static string requestString(const string &url, const string &m, const string &body, map<string, string> headers, bool &secure, string &schemeHostOut, string &host, int &port)
    {
        const string https = "https";
        secure = (url.compare(0, https.length(), https) == 0);
        const string schemeLimiter = "://";
        auto schemePos = url.find(schemeLimiter);
        
        if (schemePos == string::npos) {
            return string();
        }
        schemePos += schemeLimiter.length();
        const string slash = "/";
        auto slashPos = url.find(slash, schemePos);
        string path;
        if (slashPos == string::npos) {
            slashPos = url.length();
        } else {
            path = url.substr(slashPos);
        }
        host = url.substr(schemePos, slashPos - schemePos);
        
        port = 80;
        size_t colonPos = host.find(":");
        if (colonPos != string::npos) {
            string portStr = host.substr(colonPos+1);
            host = host.substr(0, colonPos);
            port = atoi(portStr.c_str());
        }
        
        schemeHostOut = url.substr(0, slashPos);
        
        std::stringstream request;
        request << m << " " << path << " HTTP/1.1\r\n";
        string requestHost = host;
        
        {
            /*auto it = headers.find("Host");
            if (it == headers.end()) {
                headers.insert(make_pair("Host", host));
            }*/
            // on redirect we Have to override host
            headers["Host"] = host;
        }
        if (m != "GET") {
            std::stringstream s;
            s << body.length();
            headers["Content-Length"] = s.str();
        }
        
        for (auto it = headers.begin() ; it != headers.end() ; ++it) {
            request << it->first << ": " << it->second << "\r\n";
        }
        request << "\r\n";
        request << body;
        auto requestStr = request.str();
        
        return requestStr;
    }
    
    static const size_t kKeepEvictThreshold = 2500 * 1000; 
    static const size_t kKeepData = 3000 * 1000;
    
    string method doDownload(const weak_ptr<GenericHttpDownloader> &wThis, const string &realUrl)
    {
        static mutex persistedSocketsMutex;
        static std::multimap<string, shared_ptr<Socket>> persistedSockets;
        
        auto callback = [wThis](const function<void()> &f){
            auto pThis = wThis.lock();
            if (!pThis) {
                return false;
            }
          
            pThis->queue.addTask([pThis, f]{
         
                f();
            });
            return true;
        };
        
        string schemeHost;
        string host;
        int port;
        bool secure;
        
        string requestStr;
        {
            auto pThis = wThis.lock();
            if (!pThis) {
                return "";
            }
#ifdef DEBUG_FAILING
            std::cout << "start download: " << pThis->_url << " / " << realUrl << std::endl;
#endif
            
            requestStr = requestString(realUrl, pThis->_method, pThis->_body, pThis->_headers, secure, schemeHost, host, port);
            if (requestStr.empty()) {
                callback([pThis]{
                    pThis->m._failed = true;
                    pThis->_finished = true;
                    pThis->signal();
                });
                return "";
            }
        }
        
        shared_ptr<Socket> socket;
        bool working = false;
#if DEBUG
        //vector<char> debugResult;
#endif
#ifdef DEBUG_HTTP
        bool socketReused = false;
        cout << "http request: " << requestStr << endl;
#endif
        vector<char> result;
        
        while (true) {
            {
                lock_guard<mutex> l(persistedSocketsMutex);
                auto it = persistedSockets.find(schemeHost);
                if (it == persistedSockets.end()) {
                    break;
                }
                socket = it->second;
                persistedSockets.erase(it);
            }
            
            socket->write(requestStr);
            // try to read, so we can determine whether we are timeouted
            socket->read(result);
            //copy(result.begin(), result.end(), back_inserter(debugResult));
            if (!socket->finished()) {
#ifdef DEBUG_HTTP
                cout << "http socket reused" << endl;
                socketReused = true;
#endif
                working = true;
            } else {
#ifdef DEBUG_HTTP
                cout << "http socket reuse failed" << endl;
#endif
                //lock_guard<mutex> l(persistedSocketsMutex);
                
            }
        }
        
        if (!working) {
            int tries = 0;
            do {
                ++tries;
                if (secure) {
                    socket = shared_ptr<Socket>(new SSLSocket(host));
                } else {
                    socket = shared_ptr<Socket>(new Socket(host, port));
                }
                socket->connect();
            } while (socket->finished() && tries < 3);
#ifdef DEBUG
            if (socket->finished()) {
                std::cout << "connection failed before sending actual request to " << host << std::endl;
            }
#endif
            socket->write(requestStr);
        }
        
        if (socket->finished()) {
            
            callback([wThis,requestStr]{
                auto pThis = wThis.lock();
                if (!pThis) {
                    return;
                }
#ifdef DEBUG
                std::cout << "connection closed. request string: " << requestStr << std::endl;
#endif
                pThis->m._failed = true;
                pThis->_finished = true;
                pThis->signal();
            });
            return "";
        }
        
        bool receivingData = false;
        string currentHeaderLine;
        long contentLength = -1;
        bool chunked = false;
        long receivedData = 0;
        string chunkHeader;
        bool receivingChunkHeader = false;
        bool chunksFinished = false;
        bool finished = false;
        bool closeConnection = false;
#if DEBUG
        //long totalReceived = 0;
        //vector<string> receivedHeaders;
#endif
        int resultCode = 0;
        string redirectLocation;
        
        while (true) {
            if (result.empty()) {
                
                while (true) {
                    auto pThis = wThis.lock();
                    // don't keep more than 1 mbyte data in memory
                    if (!pThis) {
                        return "";
                    }
                    if (pThis->_payload->size() > kKeepData) {
                        
                        {
                            unique_lock<mutex> l(pThis->_shallContinueMutex);
                            pThis->_shallContinue.wait_for(l, CHRONO_NS::milliseconds(1000));
                        }
                        
                        if (!callback([]{})) {
                            return "";
                        }
                    } else {
                        break;
                    }
                }
                
                socket->read(result);
                
                //copy(result.begin(), result.end(), back_inserter(debugResult));
            }
//#ifdef DEBUG_HTTP
#ifdef DEBUG_HTTP
            //if (!receivingData) {
            if (realUrl.find("videoplayback") != string::npos) {
                cout << "http got (" << realUrl << "): " << result.size() << endl;//string(result.data(), result.size()) << endl;
            }
#endif
            
            if (socket->failed()) {
                callback([wThis]{
                    auto pThis = wThis.lock();
                    if (pThis) {
                        pThis->m._failed = true;
                        pThis->_finished = true;
                        pThis->signal();
                    }
                });
                return "";
            } else if (socket->finished() && result.empty()) {
                //string debugString(debugResult.data(), debugResult.size());
                // debug here if still have problems!
                break;
            }
            
            long resultPos = 0;
            while (!receivingData && (result.size() - resultPos) > 0) {
                auto it = find(result.begin() + resultPos, result.end(), '\n');
                bool wholeLine = false;
                if (it == result.end()) {
                    currentHeaderLine.append(string(result.data() + resultPos, std::distance(result.begin() + resultPos, it)));
                    result.clear();
                    resultPos = 0;
                } else {
                    wholeLine = true;
                    currentHeaderLine.append(string(result.data() + resultPos, std::distance(result.begin() + resultPos, it) + 1));
                    
                    resultPos = std::distance(result.begin(), it) + 1;
                }
                
                if (wholeLine) {
                    if (currentHeaderLine.size() == 2) {
                        // let's rock
                        receivingData = true;
                        break;
                    }
                    
                    while (!currentHeaderLine.empty() && (currentHeaderLine.at(currentHeaderLine.size() -1) == '\r' || currentHeaderLine.at(currentHeaderLine.size() -1) == '\n')) {
                        currentHeaderLine.resize(currentHeaderLine.size() -1);
                    }
                    
                    if (resultCode == 0) {
                        auto firstSpace = currentHeaderLine.find(' ');
                        if (firstSpace != string::npos) {
                            auto secondSpace = currentHeaderLine.find(' ');
                            resultCode = atoi(currentHeaderLine.substr(firstSpace, secondSpace).c_str());
                        }
                    } else {
                        
                        auto colon = currentHeaderLine.find(':');
                        if (colon != string::npos) {
                            auto key = currentHeaderLine.substr(0, colon);
                            auto value = currentHeaderLine.substr(colon + 1, currentHeaderLine.size() - (colon +1));
                            if (value.size() > 0 && value[0] == ' ') {
                                // strip frequent whitespace
                                value.erase(0, 1);
                            }
#if DEBUG
                            //receivedHeaders.push_back(currentHeaderLine);
#endif
                            if (Base::toLower(key) == "content-length") {
                                contentLength = atol(value.c_str());
                                auto pThis = wThis.lock();
                                if (!pThis) {
                                    return "";
                                }
                                //pThis->setTotalLength(contentLength - pThis->_startPosition);
                                pThis->setTotalLength(contentLength);
                            } else if (Base::toLower(key) == "transfer-encoding") {
                                if (value.find("chunked") != string::npos) {
                                    chunked = true;
                                    receivingChunkHeader = true;
                                }
                            } else if (Base::toLower(key) == "connection") {
                                if (value.find("close") != string::npos) {
                                    closeConnection = true;
                                }
                            } else if (Base::toLower(key) == "location") {
                                redirectLocation = value;
                            }
                        }
                    }
                    currentHeaderLine.clear();
                }
            }
            
            while (receivingData && (result.size() - resultPos) > 0) {
                
                if (receivingChunkHeader) {
                    if (!chunksFinished) {
                        long i;
                        for (i = resultPos ; i < result.size() ; ++i) {
                            char c = result[i];
                            
                            chunkHeader += c;
                            //chunk header is '\r\nd3F\r\n', the first two characters are omitted in the first chunk
                            if (chunkHeader.length() > 2 && c == '\n') {
                                receivingChunkHeader = false;
                                break;
                            }
                        }
                        if (receivingChunkHeader) {
                            resultPos = result.size();
                        } else {
                            resultPos = i+1;
                            for (int j = 0 ; j < chunkHeader.length() ; j++) {
                                if (chunkHeader[j] == '\r' || chunkHeader[j] == '\n') {
                                    chunkHeader[j] = ' ';
                                }
                            }
                            contentLength = strtol(chunkHeader.c_str(), NULL, 16);
                            receivedData = 0;
                            chunkHeader.clear();
                            if (contentLength == 0) {
                                chunksFinished = true;
                                // don't break yet, we still want to receive list of trailing headers (possibly empty)
                            }
                        }
                    } else {
                        
                        long i;
                        int lineLen = 0;
                        for (i = resultPos ; i < result.size() ; ++i) {
                            char c = result[i];
                            
                            // we wait for first empty line
                            lineLen++;
                            if (c == '\n') {
                                if (lineLen == 2) {
                                    finished = true;
                                    break;
                                } else {
                                    lineLen = 0;
                                }
                            }
                        }
                        
                        if (finished) {
                            break;
                        }
                    }
                } else {
                    receivedData += (result.size() - resultPos);
#if DEBUG
                    //totalReceived += (result.size() - resultPos);
#endif
                    long surplus = 0;
                    // surplus is not applicable if content length unknown
                    if (contentLength >= 0 && receivedData > contentLength) {
                        surplus = receivedData - contentLength;
                    }
                    
                    if (!callback([wThis, result, resultPos, surplus]{
                        
                        auto pThis = wThis.lock();
                        if (!pThis) {
                            return;
                        }
                        if (result.size() > 0) {
                            const char *start = &result.front() + resultPos;
                            const char *end = &result.front() + result.size() - surplus;

                            //int debugLength = end-start;

                            pThis->_payload.setC([&](const Chain<char> &original){
                                return original.add(start, end);
                            });
                        }
                        
                        pThis->signal();
                        
                        
                    })) {
                        return "";
                    }
                    
                    resultPos = result.size() - surplus;
                    if (contentLength >= 0 && receivedData >= contentLength) {
                        if (!chunked) {
                            finished = true;
                            break;
                        } else {
                            receivedData = 0;
                            receivingChunkHeader = true;
                        }
                    }
                }
            }
            
            if (resultCode == 302 && !redirectLocation.empty()) {
                if (!socket->finished() && !closeConnection) {
                    lock_guard<mutex> l(persistedSocketsMutex);
                    persistedSockets.insert(make_pair(schemeHost, socket));
                }
                return redirectLocation;
            }
            
            if (finished) {
                break;
            }
            
            if (socket->finished()) {
                // premature end
                callback([wThis, resultCode]{
                    auto pThis = wThis.lock();
                    if (!pThis) {
                        return;
                    }
                    if (resultCode > 0) {
                        pThis->m._failed = resultCode;
                    } else {
                        pThis->m._failed = true;
                    }
                    pThis->_finished = true;
                    pThis->signal();
                });
                return "";
            }
            result.clear();
        }
        
#ifndef DEBUG_DISABLE_REUSE
        if (!socket->finished() && !closeConnection) {
            lock_guard<mutex> l(persistedSocketsMutex);
            persistedSockets.insert(make_pair(schemeHost, socket));
        }
#endif
        
        callback([wThis]{
            auto pThis = wThis.lock();
            if (!pThis) {
                return;
            }
            
            pThis->_finished = true;
            pThis->signal();
        });
        
        return "";
    }
    
    void method download(const weak_ptr<GenericHttpDownloader> &wThis, const string &requestedUrl)
    {
        string useUrl = requestedUrl;
        do {
            useUrl = doDownload(wThis, useUrl);
#if DEBUG
            if (!useUrl.empty()) {
                cout << "\nredirection (" << useUrl << ")" << endl;
            }
#endif
        } while (!useUrl.empty());
    }
    
    void method start()
    {
        weak_ptr<GenericHttpDownloader> wThis = shared_from_this();
        bool expected = false;
        if (_started.compare_exchange_strong(expected, true)) {
            
            _startTime = CHRONO_NS::steady_clock::now();
            auto url = _url;
            
#ifdef DEBUG_FAILING
            if (_url == "http:0") {
                std::cout << "strange\n";
            }
#endif
            
            Base::BackgroundExecutor::instance().addTask([url, wThis]{
                
                download(wThis, url);
            });
        }
    }
    
    method ~GenericHttpDownloader()
    {
        signal();
    }
    
    int method offsetAvailable(int offset) const
    {
        assert(offset >= 0);
        
        if (offset < _startPosition) {
            return 0;
        }
        offset -= _startPosition;
        int available = _payload->size();
        
        if (available <= offset) {
            if (_finished) {
                // if finished but can't give anything, that means that we haven't succeeded on downloading everything
                // the following caused great trouble in the totalLength = 0, offset = available+1 case
                // m._failed = true;
                if (available == 0) {
                    m._failed = true;
                }
            }
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
        
        
        shared_ptr<const Chain<char>> payload = _payload;
        
        int dataSize = payload->size();
        int avail = dataSize - offset + _startPosition;
        if (avail < 0) {
            // e.g. finished
            avail = 0;
        }
        
		if (avail == 0) {
			localF(nullptr, 0);
		} else {
			size_t before = 0;
			auto data = payload->pieceAt(offset - _startPosition, before);

			// deliver first
			int passOffset = offset - _startPosition - data.second;
			if (data.first->empty()) {
				localF(nullptr, 0);
			}
			else {
				localF(&data.first->front() + passOffset, data.first->size() - passOffset);
			}

			// upkeep later
			if (dataSize > kKeepEvictThreshold && before > 0) {
				_payload.setC([&](const Chain<char> &chain){
					return chain.trim(before);
				});
				_startPosition += data.second;
				if (_payload->size() <= kKeepData) {
					lock_guard<mutex> l(_shallContinueMutex);
					_shallContinue.notify_all();
				}
			}
		}
    }

    void method signal()
    {
        //dispatch_semaphore_signal(semaphore);
        IAsyncData::signal();
    }
    
    bool method finished() const
    {
#ifdef DEBUG_FAILING
        if (_finished) {
//            std::cout << "finished: " << _url << std::endl;
        }
#endif
        return _finished;
    }
    
    int method failed() const
    {
#ifdef DEBUG_FAILING
        if (m._failed > 0) {
            //std::cout << "fail: " << _url << " code: " << m._failed << std::endl;
        }
#endif
        
        return m._failed;
    }
}
