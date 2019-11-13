/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ReqMsgDecoder_h
#define __thomsonreuters_ema_access_ReqMsgDecoder_h

#include "EmaPool.h"
#include "MsgDecoder.h"
#include "OmmQos.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class ReqMsgDecoder : public MsgDecoder
{
public :

	ReqMsgDecoder();

	~ReqMsgDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

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

	Int32 getStreamId() const;

	UInt16 getDomainType() const;

	const EmaString& getName() const;

	UInt8 getNameType() const;

	UInt32 getServiceId() const;

	const EmaString& getServiceName() const;

	Int32 getId() const;

	UInt32 getFilter() const;

	const EmaBuffer& getExtendedHeader() const;

	bool hasPriority() const;

	bool hasQos() const;

	bool hasView() const;

	bool hasBatch() const;

	UInt8 getPriorityClass() const;

	UInt16 getPriorityCount() const;

	UInt32 getTimeliness() const;

	UInt32 getRate() const;

	bool getInitialImage() const;

	bool getInterestAfterRefresh() const;

	bool getConflatedInUpdates() const;

	bool getPause() const;

	bool getPrivateStream() const;

	void setServiceName( const char* , UInt32 , bool nullTerm = true );

	const EmaBuffer& getHexBuffer() const;

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	void setQosInt();

	mutable EmaStringInt			_name;

	mutable EmaStringInt			_serviceName;

	mutable EmaBufferInt			_extHeader;

	OmmQos							_qos;

	mutable EmaBufferInt			_hexBuffer;

	mutable bool					_serviceNameSet;

	OmmError::ErrorCode				_errorCode;

	friend class ReqMsg;
};

class ReqMsgDecoderPool : public DecoderPool< ReqMsgDecoder >
{
public :

	ReqMsgDecoderPool( unsigned int size = 5 ) : DecoderPool< ReqMsgDecoder >( size ) {};

	~ReqMsgDecoderPool() {}

private :

	ReqMsgDecoderPool( const ReqMsgDecoderPool& );
	ReqMsgDecoderPool& operator=( const ReqMsgDecoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ReqMsgDecoder_h
