/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_PostMsgDecoder_h
#define __thomsonreuters_ema_access_PostMsgDecoder_h

#include "EmaPool.h"
#include "MsgDecoder.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class PostMsgDecoder : public MsgDecoder
{
public :

	PostMsgDecoder();

	~PostMsgDecoder();

	void setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	void setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	void setRsslData( RsslDecodeIterator* , RsslBuffer* );

	bool hasMsgKey() const;

	bool hasName() const;

	bool hasNameType() const;

	bool hasServiceId() const;

	bool hasServiceName() const;

	bool hasId() const;

	bool hasPostId() const;

	bool hasPostUserRights() const;

	bool hasFilter() const;

	bool hasAttrib() const;

	bool hasPayload() const;

	bool hasHeader() const;

	bool hasSeqNum() const;

	bool hasSecondarySeqNum() const;

	bool hasPartNum() const;

	bool hasPermissionData() const;

	Int32 getStreamId() const;

	UInt16 getDomainType() const;

	const EmaString& getName() const;

	const EmaString& getServiceName() const;

	UInt8 getNameType() const;

	UInt32 getServiceId() const;

	Int32 getId() const;

	UInt32 getFilter() const;

	UInt32 getSeqNum() const;

	UInt32 getPublisherIdUserId() const;

	UInt32 getPublisherIdUserAddress() const;

	UInt32 getPostId() const;

	UInt16 getPostUserRights() const;

	UInt16 getPartNum() const;

	bool getSolicitAck() const;

	bool getComplete() const;

	const EmaBuffer& getPermissionData() const;

	const EmaBuffer& getHeader() const;

	const EmaBuffer& getHexBuffer() const;

private :

	RsslMsg								_rsslMsg;

	RsslMsg*							_pRsslMsg;

	mutable EmaStringInt				_name;

	mutable EmaBufferInt				_extHeader;

	mutable EmaBufferInt				_permission;

	mutable EmaStringInt				_serviceName;

	mutable EmaBufferInt				_hexBuffer;

	mutable bool						_serviceNameSet;

	UInt8								_rsslMajVer;

	UInt8								_rsslMinVer;

	mutable OmmError::ErrorCode	_errorCode;
};

class PostMsgDecoderPool : public DecoderPool< PostMsgDecoder >
{
public :

	PostMsgDecoderPool( unsigned int size = 5 ) : DecoderPool< PostMsgDecoder >( size ) {};

	~PostMsgDecoderPool() {}

private :

	PostMsgDecoderPool( const PostMsgDecoderPool& );
	PostMsgDecoderPool& operator=( const PostMsgDecoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_PostMsgDecoder_h
