/*
 * DecoderAv.h
 *
 *  Created on: Oct 16, 2013
 *      Author: zsszatmari
 */

#ifndef DECODERAV_H_
#define DECODERAV_H_

#include "Environment.h"

#ifndef DISABLE_DECODERAV

#include "stdplus.h"
#include SHAREDFROMTHIS_H
#include "IDecoder.h"
#include "SerialExecutor.h"

namespace Gear
{
    class DecoderAvWorker;
    
	class DecoderAv final : public IDecoder, public MEMORY_NS::enable_shared_from_this<DecoderAv>
	{
	public:
		static shared_ptr<DecoderAv> create();
		~DecoderAv();

		virtual Status readInto(unsigned char *outData, size_t outSize,size_t &producedBytes);
		virtual void feedFrom(unsigned const char *inData, size_t inSize);
		virtual long seekOffset(float time, size_t guess);
		virtual void setDiscontinous();
		virtual void setTotalLength(int64_t totalLength);

	private:
        shared_ptr<DecoderAvWorker> _worker;

		DecoderAv();

		bool analyzed;
	};

} /* namespace Gear */

#endif

#endif /* DECODERAV_H_ */
