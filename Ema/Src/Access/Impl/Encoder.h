/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_Encoder_h
#define __thomsonreuters_ema_access_Encoder_h

#include "Data.h"
#include "rtr/rsslDataTypeEnums.h"
#include "EncodeIterator.h"

namespace thomsonreuters {

namespace ema {

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

	UInt8 convertDataType( DataType::DataTypeEnum );

	bool isComplete() const;

protected :

	Encoder();

	virtual ~Encoder();

	void acquireEncIterator();

	void releaseEncIterator();

	bool hasEncIterator();

	EncodeIterator*		_pEncodeIter;

	const Encoder*		_iteratorOwner;

	bool				_containerComplete;
};

}

}

}

#endif // __thomsonreuters_ema_access_Encoder_h
