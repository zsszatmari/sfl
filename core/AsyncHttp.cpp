#include <iostream>
#include "AsyncHttp.h"
#include "HttpDownloader.h"
#include "BackgroundExecutor.h"

using namespace Gear;
using namespace Base;

namespace AsyncHttp
{
	void download(boost::asio::io_service &io, const Request &request, const std::function<void(const Response &)> &response)
	{
		BackgroundExecutor::instance().addTask([&io,request,response]{
        	auto downloader = HttpDownloader::create(request.url, request.headers, request.httpMethod, request.body);
        	downloader->waitUntilFinished();

        	io.dispatch([downloader,response]{
        		response(Response(downloader->failed() ? downloader->failed() : 200, *downloader));
        	});		
		});
	}

	void download(boost::asio::io_service &io, const Request &request, int chunkSize, const std::function<void(const Response &)> &response)
	{
		BackgroundExecutor::instance().addTask([&io,request,response,chunkSize]{
        	auto downloader = HttpDownloader::create(request.url, request.headers, request.httpMethod, request.body);
        	
			int pos = 0;
			bool shouldContinue = true;
			while (shouldContinue) {
				auto prevPos = pos;
				pos += chunkSize;
				downloader->waitSync(pos);
				downloader->accessChunk(prevPos, [&](const char *ptr, int avail){

					//std::cout << "downloading... " << prevPos << "-" << pos << " (" << avail << ")" << std::endl;
					if (avail > chunkSize) {
						avail = chunkSize;
					}
					vector<char> result = vector<char>(ptr, ptr+avail);
					if (avail == 0) {
						//std::cout << "no more available, bailing out\n";
						shouldContinue = false;
					} else {
						io.dispatch([response,result]{
			        		response(Response(200, result));
			        	});	
					}
				});

				// this is bad: it may be that all data has been downloaded, we simply haven't accessed it yet
				/*
				if (downloader->finished()) {
					std::cout << "finished, bailing out\n";
					shouldContinue = false;
				}*/
			}

			bool code = downloader->failed() ? downloader->failed() : 200;
			io.dispatch([code,response]{
        		response(Response(code, vector<char>()));
        	});		
		});
	}


	Request::Request(const std::string &aUrl, const std::map<std::string,std::string> &aHeaders, const std::string &aMethod, const std::string &aBody) :
		url(aUrl),
		headers(aHeaders),
		httpMethod(aMethod),
		body(aBody)
	{
	}

	Response::Response(int aStatusCode, const std::vector<char> &aBody) :
		statusCode(aStatusCode),
		body(aBody)
	{
	}

	bool Response::ok() const
	{
		return statusCode == 200;
	}

	bool Response::timeout() const
	{
		// unsupported as of now, but will be a custom status code
		return false;
	}

	std::string Response::bodyAsString() const
	{
		return std::string(body.begin(),body.end());
	}
}