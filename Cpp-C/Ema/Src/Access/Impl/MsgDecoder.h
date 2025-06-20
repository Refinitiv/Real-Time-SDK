/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_MsgDecoder_h
#define __refinitiv_ema_access_MsgDecoder_h

#include "Decoder.h"
#include "NoDataImpl.h"

namespace refinitiv {

namespace ema {

namespace access {

class Msg;

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

	const RsslDataDictionary* getRsslDictionary();

	UInt8 getMajorVersion();

	UInt8 getMinorVersion();

	RsslBuffer& getCopiedBuffer();

	void cloneBufferToMsg(Msg* sourceMsg, const char* functionName);

	void deallocateCopiedBuffer();

protected :

	MsgDecoder();

	virtual ~MsgDecoder();

	void cloneMsgKey(const Msg& other, RsslMsgKey* destMsgKey);

	const RsslDataDictionary*		_pRsslDictionary;

	UInt8							_rsslMajVer;

	UInt8							_rsslMinVer;

	NoDataImpl						_attrib;

	NoDataImpl						_payload;

	RsslBuffer						_copiedBuffer;

	RsslMsg							_rsslMsg;

	RsslMsg*						_pRsslMsg;

	EmaBuffer						_nameData;

	EmaBuffer						_copiedBufferData;

	EmaBuffer						_encAttribData;
};

}

}

}

#endif //__refinitiv_ema_access_MsgDecoder_h
