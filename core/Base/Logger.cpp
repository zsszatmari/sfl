/*
 * Logger.cpp
 *
 *  Created on: Sep 5, 2013
 *      Author: zsszatmari
 */

#include "Environment.h"

#ifdef TIZEN
#include <FBase.h>
#else
#ifdef TARGET_OS_IPHONE
#include <CoreFoundation/CoreFoundation.h>
extern "C" {
    void NSLog(CFStringRef format, ...);
}
#else
#include <iostream>
#endif
#endif
#include "Logger.h"

namespace Base
{
#define method LogStream::

	method LogStream()
	{
	}

	method ~LogStream()
	{
		auto s = str.str();
		if (!s.empty()) {
			Logger::log(s);
		}
	}

	method LogStream(LogStream &rhs)
	{
		str << rhs.str.str();
		rhs.str.str(std::string());
		rhs.str.clear();
	}

	LogStream &method operator=(LogStream &rhs)
	{
		str << rhs.str.str();
		rhs.str.str(std::string());
		rhs.str.clear();
		return *this;
	}

#undef method
#define method Logger::

	void method log(std::string &str)
	{
#ifdef TIZEN
		AppLog("%s", str.c_str());
#else
#ifdef TARGET_OS_IPHONE
        NSLog(CFSTR("%s"), str.c_str());
#else
		std::cout << str << std::endl;
#endif
#endif
	}

	LogStream method stream()
	{
		LogStream stream;
		return stream;
	}
} /* namespace Base */
