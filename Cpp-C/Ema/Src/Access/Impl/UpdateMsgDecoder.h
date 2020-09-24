/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_UpdateMsgDecoder_h
#define __rtsdk_ema_access_UpdateMsgDecoder_h

#include "EmaPool.h"
#include "MsgDecoder.h"
#include "UpdateMsg.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace rtsdk {

namespace ema {

namespace access {

class UpdateMsgDecoder : public MsgDecoder
{
public :

	UpdateMsgDecoder();

	~UpdateMsgDecoder();

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

	bool hasSeqNum() const;

	bool hasPermissionData() const;

	bool hasConflated() const;

	bool hasPublisherId() const;

	UInt8 getUpdateTypeNum() const;

	UInt32 getSeqNum() const;

	const EmaBuffer& getPermissionData() const;

	UInt16 getConflatedTime() const;

	UInt16 getConflatedCount() const;

	UInt32 getPublisherIdUserId() const;

	UInt32 getPublisherIdUserAddress() const;

	bool getDoNotCache() const;

	bool getDoNotConflate() const;

	bool getDoNotRipple() const;

	void setServiceName( const char* , UInt32 , bool nullTerm = true );

	const EmaBuffer& getHexBuffer() const;

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	mutable EmaStringInt		_name;

	mutable EmaStringInt		_serviceName;

	mutable EmaBufferInt		_extHeader;

	mutable EmaBufferInt		_permission;

	mutable EmaBufferInt		_hexBuffer;

	mutable bool				_serviceNameSet;

	OmmError::ErrorCode			_errorCode;

	friend class UpdateMsg;
};

class UpdateMsgDecoderPool : public DecoderPool< UpdateMsgDecoder >
{
public :

	UpdateMsgDecoderPool( unsigned int size = 5 ) : DecoderPool< UpdateMsgDecoder >( size ) {};

	~UpdateMsgDecoderPool() {}

private :

	UpdateMsgDecoderPool( const UpdateMsgDecoderPool& );
	UpdateMsgDecoderPool& operator=( const UpdateMsgDecoderPool& );
};

}

}

}

#endif // __rtsdk_ema_access_UpdateMsgDecoder_h
