/*
 * DecoderPcm.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author: zsszatmari
 */

#include "DecoderPcm.h"
#include MEMORY_H

namespace Gear
{
#define method DecoderPcm::

	shared_ptr<DecoderPcm> method create()
	{
		return shared_ptr<DecoderPcm>(new DecoderPcm());
	}

	method DecoderPcm() :
		_buffer(1024)
	{
	}

	IDecoder::Status method readInto(unsigned char *outData, size_t outSize,size_t &producedBytes)
	{
		int available = 0;
		void *buf = _buffer.tail(available);
		producedBytes = available;
		if (producedBytes > outSize) {
			producedBytes = outSize;
		}

		memcpy(outData, buf, producedBytes);
		_buffer.consume(producedBytes);

		return (producedBytes > 0) ? IDecoder::Status::NoError : IDecoder::Status::NeedMore;
	}

	void method feedFrom(unsigned const char *inData, size_t inSize)
	{
		int available = 0;
		void *buf = _buffer.head(available);
		if (available > inSize) {
			available = inSize;
		}

		memcpy(buf, inData, available);
		_buffer.produce(available);
	}

	long method seekOffset(float time, size_t guess)
	{
		return time * 2 * 2 * 44100;
	}

	void method setDiscontinous()
	{
	}


} /* namespace Gear */
