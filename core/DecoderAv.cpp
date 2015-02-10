/*
 * DecoderAv.cpp
 *
 *  Created on: Oct 16, 2013
 *      Author: zsszatmari
 */

#include <iostream>
#include "DecoderAv.h"
#include "DecoderAvWorker.h"

#ifndef DISABLE_DECODERAV

namespace Gear
{

#define method DecoderAv::

    shared_ptr<DecoderAv> method create()
	{
		shared_ptr<DecoderAv> pThis(new DecoderAv());
		return pThis;
	}

	method DecoderAv() :
        _worker(DecoderAvWorker::create())
		//convertContext(nullptr)
	{
        _worker->start();
	}

	method ~DecoderAv()
	{
        _worker->release();
	}

	IDecoder::Status method readInto(unsigned char *outData, size_t outSize, size_t &producedBytes)
	{
        return _worker->readInto(outData, outSize, producedBytes);
	}

	void method feedFrom(unsigned const char *inData, size_t inSize)
	{
        _worker->feedFrom(inData, inSize);
    }

	long method seekOffset(float time, size_t guess)
	{
        return _worker->seekOffset(time, guess);
	}

	void method setTotalLength(int64_t aTotalLength)
	{
		_worker->setTotalLength(aTotalLength);
	}

	void method setDiscontinous()
	{
	}

} /* namespace Gear */

#endif
