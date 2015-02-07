/*
 * Logger.h
 *
 *  Created on: Sep 5, 2013
 *      Author: zsszatmari
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <sstream>
#include "Environment.h"

namespace Base {

	class LogStream final
	{
	public:
		LogStream();
		~LogStream();
		LogStream(LogStream &rhs);
		LogStream &operator=(LogStream &rhs);

		template<typename T>
		LogStream &operator<<(const T rhs)
		{
			str << rhs;
			return *this;
		}

	private:
		std::stringstream str;
	};

	class Logger final
	{
	public:
		static void log(std::string &str);
		static LogStream stream();
	};

} /* namespace Base */

#endif /* LOGGER_H_ */
