/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_Encoder_h
#define __refinitiv_ema_access_Encoder_h

#include "Data.h"
#include "rtr/rsslDataTypeEnums.h"
#include "EncodeIterator.h"

namespace rtsdk {

namespace ema {

namespace rdm  {

	class DataDictionaryImpl;
}

namespace access {

class EmaString;

class Encoder
{
public :

	virtual void clear();

	bool ownsIterator() const;

	void passEncIterator( Encoder& );

	virtual RsslBuffer& getRsslBuffer() const;

	RsslUInt8 getMajVer() const;

	RsslUInt8 getMinVer() const;

	virtual void endEncodingEntry() const = 0;

	UInt8 convertDataType( DataType::DataTypeEnum ) const;

	virtual bool isComplete() const;

protected :

	friend class rtsdk::ema::rdm::DataDictionaryImpl;

	Encoder();

	virtual ~Encoder();

	void acquireEncIterator(UInt32 allocatedSize = 4096);

	void releaseEncIterator();

	bool hasEncIterator() const;

	EncodeIterator*		_pEncodeIter;

	const Encoder*		_iteratorOwner;

	bool				_containerComplete;
};

}

}

}

#endif // __refinitiv_ema_access_Encoder_h
