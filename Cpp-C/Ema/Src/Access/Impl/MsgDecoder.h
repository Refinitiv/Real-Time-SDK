/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_MsgDecoder_h
#define __thomsonreuters_ema_access_MsgDecoder_h

#include "Decoder.h"
#include "NoDataImpl.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class MsgDecoder : public Decoder
{
public :

	const Data& getAttribData() const;

	const Data& getPayloadData() const;

	virtual bool hasMsgKey() const = 0;

	virtual bool hasName() const = 0;

	virtual bool hasNameType() const = 0;

	virtual bool hasServiceId() const = 0;

	virtual bool hasId() const = 0;

	virtual bool hasFilter() const = 0;

	virtual bool hasAttrib() const = 0;

	virtual bool hasPayload() const = 0;

	virtual bool hasExtendedHeader() const = 0;

	virtual Int32 getStreamId() const = 0;

	virtual UInt16 getDomainType() const = 0;

	virtual const EmaString& getName() const = 0;

	virtual UInt8 getNameType() const = 0;

	virtual UInt32 getServiceId() const = 0;

	virtual Int32 getId() const = 0;

	virtual UInt32 getFilter() const = 0;

	virtual const EmaBuffer& getExtendedHeader() const = 0;

	void setAtExit();

protected :

	MsgDecoder();

	virtual ~MsgDecoder();

	const RsslDataDictionary*		_pRsslDictionary;

	NoDataImpl						_attrib;

	NoDataImpl						_payload;
};

}

}

}

#endif //__thomsonreuters_ema_access_MsgDecoder_h
