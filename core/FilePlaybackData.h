/*
 * FilePlaybackData.h
 *
 *  Created on: Nov 6, 2013
 *      Author: zsszatmari
 */

#ifndef FILEPLAYBACKDATA_H_
#define FILEPLAYBACKDATA_H_

#include <string>
#include "IPlaybackData.h"

namespace Gear 
{
	using std::function;

	class FilePlaybackData : public IPlaybackData
	{
	public:
		FilePlaybackData(const std::string &path, const Format format, bool offlineEncoded = false);
		virtual ~FilePlaybackData();

        virtual void waitAsync(int offset, const function<void()> &f);

		virtual int offsetAvailable(int offset) const;

		virtual void accessChunk(int offset, const function<void(const char *ptr, int available)> &f);
		virtual void seek(int offset);

		virtual int totalLength() const;
		virtual bool finished() const;
		virtual int failed() const;

	private:
		int _failed;
		FILE *_handle;
		long _totalLength;
		long _currentPos;
        
        std::vector<char> _buffer;
        int _bufferPos;
        bool _offlineEncoded;
	};

} /* namespace Gear */
#endif /* FILEPLAYBACKDATA_H_ */
