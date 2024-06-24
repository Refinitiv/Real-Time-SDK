/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|         Copyright (C) 2019 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_AckMsgDecoder_h
#define __refinitiv_ema_access_AckMsgDecoder_h

#include "EmaPool.h"
#include "MsgDecoder.h"
#include "AckMsg.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace refinitiv {

namespace ema {

namespace access {

class AckMsgDecoder : public MsgDecoder
{
public :

	AckMsgDecoder();

	~AckMsgDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg*, const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer*, const RsslDataDictionary*, void* );

	bool setRsslData( RsslDecodeIterator*, RsslBuffer* );

	bool hasMsgKey() const;

	bool hasName() const;

	bool hasNameType() const;

	bool hasServiceId() const;

	bool hasServiceName() const;

	bool hasId() const;

	bool hasFilter() const;

	bool hasAttrib() const;

	bool hasPayload() const;

	bool hasExtendedHeader() const;

	bool hasSeqNum() const;

	bool hasNackCode() const;

	bool hasText() const;

	Int32 getStreamId() const;

	UInt16 getDomainType() const;

	const EmaString& getName() const;

	UInt8 getNameType() const;

	UInt32 getServiceId() const;

	const EmaString& getServiceName() const;

	Int32 getId() const;

	UInt32 getFilter() const;

	UInt32 getAckId() const;

	UInt32 getSeqNum() const;

	const EmaString& getText() const;

	UInt8 getNackCode() const;

	const EmaBuffer& getExtendedHeader() const;

	const EmaBuffer& getHexBuffer() const;

	bool getPrivateStream() const;

	void setServiceName( const char*, UInt32 , bool nullTerm = true );

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

	friend class AckMsg;

private :

	mutable EmaStringInt	_name;

	mutable EmaStringInt	_serviceName;

	mutable EmaStringInt	_text;

	mutable EmaBufferInt	_extHeader;

	mutable EmaBufferInt	_hexBuffer;

	mutable bool			_serviceNameSet;

	mutable OmmError::ErrorCode	_errorCode;
};

class AckMsgDecoderPool : public DecoderPool< AckMsgDecoder >
{
public :

	AckMsgDecoderPool( unsigned int size = 5 ) : DecoderPool< AckMsgDecoder >( size ) {};

	~AckMsgDecoderPool() {}

private :

	AckMsgDecoderPool( const AckMsgDecoderPool& );
	AckMsgDecoderPool& operator=( const AckMsgDecoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_AckMsgDecoder_h
