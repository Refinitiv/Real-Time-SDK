/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_RefreshMsgDecoder_h
#define __thomsonreuters_ema_access_RefreshMsgDecoder_h

#include "EmaPool.h"
#include "MsgDecoder.h"
#include "RefreshMsg.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace rtsdk {

namespace ema {

namespace access {

class RefreshMsgDecoder : public MsgDecoder
{
public :

	RefreshMsgDecoder();

	~RefreshMsgDecoder();

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

	bool hasQos() const;

	bool hasSeqNum() const;

	bool hasPartNum() const;

	bool hasPermissionData() const;

	bool hasPublisherId() const;

	const OmmState& getState() const;

	const OmmQos& getQos() const;

	UInt32 getSeqNum() const;

	UInt16 getPartNum() const;

	const EmaBuffer& getItemGroup() const;

	const EmaBuffer& getPermissionData() const;

	UInt32 getPublisherIdUserId() const;

	UInt32 getPublisherIdUserAddress() const;

	bool getDoNotCache() const;

	bool getSolicited() const;

	bool getComplete() const;

	bool getClearCache() const;

	bool getPrivateStream() const;

	void setServiceName( const char* , UInt32 , bool nullTerm = true );

	const EmaBuffer& getHexBuffer() const;

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	void setStateInt() const;

	void setQosInt() const;

	mutable EmaStringInt			_name;

	mutable EmaStringInt			_serviceName;

	mutable EmaBufferInt			_extHeader;

	mutable EmaBufferInt			_permission;

	mutable EmaBufferInt			_itemGroup;

	mutable OmmState				_state;

	mutable OmmQos					_qos;

	mutable EmaBufferInt			_hexBuffer;

	mutable bool					_serviceNameSet;
	
	mutable bool					_stateSet;

	mutable bool					_qosSet;

	OmmError::ErrorCode				_errorCode;

	friend class RefreshMsg;
};

class RefreshMsgDecoderPool : public DecoderPool< RefreshMsgDecoder >
{
public :

	RefreshMsgDecoderPool( unsigned int size = 5 ) : DecoderPool< RefreshMsgDecoder >( size ) {};

	~RefreshMsgDecoderPool() {}

private :

	RefreshMsgDecoderPool( const RefreshMsgDecoderPool& );
	RefreshMsgDecoderPool& operator=( const RefreshMsgDecoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_RefreshMsgDecoder_h
