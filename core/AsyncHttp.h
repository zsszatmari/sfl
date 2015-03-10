#ifndef ASYNCHTTP_ASYNCHTTP_H
#define ASYNCHTTP_ASYNCHTTP_H

#include "boost/asio.hpp"

namespace AsyncHttp
{
	struct Request final
	{
		Request(const std::string &url, const std::map<std::string,std::string> &headers = std::map<std::string,std::string>(), const std::string &m = "GET", const std::string &body = "");	

		std::string url;
		std::map<std::string,std::string> headers;
		std::string httpMethod;
		std::string body;
	};

	struct Response final
	{
		Response(int aStatusCode, const std::vector<char> &body);

		bool ok() const;
		bool timeout() const;
		std::string bodyAsString() const;

		int statusCode;
		std::vector<char> body;
	};

	void download(boost::asio::io_service &io, const Request &request, const std::function<void(const Response &)> &response);
	// downlaods a large file in chunks. A final zero-sized chunk represents tehe end of stream
	void download(boost::asio::io_service &io, const Request &request, int chunkSize, const std::function<void(const Response &)> &response);
}

#endif