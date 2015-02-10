/*
 * DecoderPcm.h
 *
 *  Created on: Oct 24, 2013
 *      Author: zsszatmari
 */

#ifndef DECODERPCM_H_
#define DECODERPCM_H_

#include "stdplus.h"
#include "IDecoder.h"
#include "CircularBuffer.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear {

	class DecoderPcm final : public IDecoder
	{
	public:
		static shared_ptr<DecoderPcm> create();

		virtual IDecoder::Status readInto(unsigned char *outData, size_t outSize,size_t &producedBytes);
		virtual void feedFrom(unsigned const char *inData, size_t inSize);
		virtual long seekOffset(float time, size_t guess);
		virtual void setDiscontinous();

	private:
		DecoderPcm();

		CircularBuffer _buffer;
	};

} /* namespace Gear */
#endif /* DECODERPCM_H_ */
