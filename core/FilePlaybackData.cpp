/*
 * FilePlaybackData.cpp
 *
 *  Created on: Nov 6, 2013
 *      Author: zsszatmari
 */

#include <iostream>
#include "FilePlaybackData.h"
#include "OfflineStorage.h"

namespace Gear
{
	using std::string;

#define method FilePlaybackData::

	method FilePlaybackData(const string &path, const Format format, bool offlineEncoded) :
        IPlaybackData(format),
        _failed(0),
		_totalLength(0),
		_currentPos(0),
        _bufferPos(0),
        _offlineEncoded(offlineEncoded)
	{
		_handle = fopen(path.c_str(), "r");
		if (!_handle) {
			_failed = 1;
		} else {
			fseek(_handle, 0, SEEK_END);
			_totalLength = ftell(_handle);
			fseek(_handle, 0, SEEK_SET);
		}
	}

	method ~FilePlaybackData()
	{
		if (_handle) {
			fclose(_handle);
		}
	}

    void method waitAsync(int offset, const function<void()> &f)
    {
    	f();
    }

	int method offsetAvailable(int offset) const
	{
		return _totalLength - offset;
	}

	void method accessChunk(int offset, const function<void(const char *ptr, int available)> &f)
	{
		if (!_handle) {
			return;
		}
        if (offset >= _bufferPos && offset < _bufferPos + _buffer.size()) {
            f(_buffer.data() + (offset - _bufferPos), _buffer.size() - (offset - _bufferPos));
            return;
        }
        
		if (_currentPos != offset) {
            int result = fseek(_handle, offset, SEEK_SET);
            //std::cout << "seeking to: " << offset << "/" << _totalLength << " success: " << result << std::endl;
			_currentPos = offset;
		}

        _buffer.resize(8192);
        _bufferPos = _currentPos;
		size_t read = fread(_buffer.data(), 1, _buffer.size(), _handle);
        _buffer.resize(read);
        if (_offlineEncoded) {
        	OfflineStorage::decode(_buffer.data(), read, offset);
        }
		f(_buffer.data(), read);
		_currentPos += read;
	}

	void method seek(int offset)
	{
	}

	int method totalLength() const
	{
		return _totalLength;
	}

	bool method finished() const
	{
		return true;
	}

	int method failed() const
	{
		return _failed;
	}

} /* namespace Gear */
