/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "PostMsgEncoder.h"
#include "ComplexType.h"

using namespace thomsonreuters::ema::access;

PostMsgEncoder::PostMsgEncoder() :
 MsgEncoder()
#ifdef __EMA_COPY_ON_SET__
 ,_permissionData()
 ,_permissionDataSet( false )
#else
 ,_pPermissionData( 0 )
#endif
{
	rsslClearPostMsg( &_rsslPostMsg );
	_rsslPostMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslPostMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

PostMsgEncoder::~PostMsgEncoder()
{
}

void PostMsgEncoder::clear()
{
	MsgEncoder::clear();

#ifdef __EMA_COPY_ON_SET__
	_permissionDataSet = false;
#else
	_pPermissionData = 0;
#endif

	rsslClearPostMsg( &_rsslPostMsg );
	_rsslPostMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslPostMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

void PostMsgEncoder::extendedHeader( const EmaBuffer& extHeader )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_extendedHeader = extHeader;
	_rsslPostMsg.extendedHeader.data = (char*)_extendedHeader.c_buf();
	_rsslPostMsg.extendedHeader.length = _extendedHeader.length();
#else
	_rsslPostMsg.extendedHeader.data = (char*)extHeader.c_buf();
	_rsslPostMsg.extendedHeader.length = extHeader.length();
#endif

	rsslPostMsgApplyHasExtendedHdr( &_rsslPostMsg );
}

void PostMsgEncoder::permissionData( const EmaBuffer& permData )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_permissionData = permData;
	_rsslPostMsg.permData.data = (char*)_permissionData.c_buf();
	_rsslPostMsg.permData.length = _permissionData.length();
#else
	_rsslPostMsg.permData.data = (char*)permData.c_buf();
	_rsslPostMsg.permData.length = permData.length();
#endif

	rsslPostMsgApplyHasPermData( &_rsslPostMsg );
}

void PostMsgEncoder::seqNum( UInt32 seqNum )
{
	acquireEncIterator();

	_rsslPostMsg.seqNum = seqNum;
	rsslPostMsgApplyHasSeqNum( &_rsslPostMsg );
}

void PostMsgEncoder::partNum( UInt16 partNum )
{
	acquireEncIterator();

	_rsslPostMsg.partNum = partNum;
	rsslPostMsgApplyHasPartNum( &_rsslPostMsg );
}

void PostMsgEncoder::postUserRights( UInt16 postUserRights )
{
	acquireEncIterator();

	_rsslPostMsg.postUserRights = postUserRights;
	rsslPostMsgApplyHasPostUserRights( &_rsslPostMsg );
}

void PostMsgEncoder::publisherId( UInt32 userId, UInt32 userAddress )
{
	acquireEncIterator();

	_rsslPostMsg.postUserInfo.postUserId = userId;
	_rsslPostMsg.postUserInfo.postUserAddr = userAddress;
}

void PostMsgEncoder::postId( UInt32 postId )
{
	acquireEncIterator();

	_rsslPostMsg.postId = postId;
	rsslPostMsgApplyHasPostId( &_rsslPostMsg );
}

void PostMsgEncoder::solicitAck( bool ack )
{
	acquireEncIterator();

	if ( ack )
		_rsslPostMsg.flags |= RSSL_PSMF_ACK;
	else
		_rsslPostMsg.flags &= ~RSSL_PSMF_ACK;
}

void PostMsgEncoder::complete( bool complete )
{
	acquireEncIterator();

	if ( complete )
		_rsslPostMsg.flags |= RSSL_PSMF_POST_COMPLETE;
	else
		_rsslPostMsg.flags &= ~RSSL_PSMF_POST_COMPLETE;
}

RsslPostMsg* PostMsgEncoder::getRsslPostMsg() const
{
	return (RsslPostMsg*)&_rsslPostMsg;
}

void PostMsgEncoder::name( const EmaString& name )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_name = name;
	_rsslPostMsg.msgBase.msgKey.name.data = (char*)_name.c_str();
	_rsslPostMsg.msgBase.msgKey.name.length = _name.length();
#else
	_rsslPostMsg.msgBase.msgKey.name.data = (char*)name.c_str();
	_rsslPostMsg.msgBase.msgKey.name.length = name.length();
#endif

	_rsslPostMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	_rsslPostMsg.flags |= RSSL_PSMF_HAS_MSG_KEY;
}

void PostMsgEncoder::nameType( UInt8 nameType )
{
	acquireEncIterator();

	_rsslPostMsg.msgBase.msgKey.nameType = nameType;
	_rsslPostMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
	_rsslPostMsg.flags |= RSSL_PSMF_HAS_MSG_KEY;
}

void PostMsgEncoder::filter( UInt32 filter )
{
	acquireEncIterator();

	_rsslPostMsg.msgBase.msgKey.filter = filter;
	_rsslPostMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
	_rsslPostMsg.flags |= RSSL_PSMF_HAS_MSG_KEY;
}

void PostMsgEncoder::addFilter( UInt32 filter )
{
	acquireEncIterator();

	_rsslPostMsg.msgBase.msgKey.filter |= filter;
	_rsslPostMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
	_rsslPostMsg.flags |= RSSL_PSMF_HAS_MSG_KEY;
}

void PostMsgEncoder::identifier( Int32 id )
{
	acquireEncIterator();

	_rsslPostMsg.msgBase.msgKey.identifier = id;
	_rsslPostMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
	_rsslPostMsg.flags |= RSSL_PSMF_HAS_MSG_KEY;
}

void PostMsgEncoder::attrib( const ComplexType& attrib )
{
	acquireEncIterator();

	_rsslPostMsg.msgBase.msgKey.attribContainerType = convertDataType( attrib.getDataType() );

#ifdef __EMA_COPY_ON_SET__
	RsslBuffer& rsslBuf = static_cast<const Data&>(attrib).getEncoder().getRsslBuffer();
	_attrib.setFrom( rsslBuf.data, rsslBuf.length );

	_rsslPostMsg.msgBase.msgKey.encAttrib.data = (char*)_attrib.c_buf();
	_rsslPostMsg.msgBase.msgKey.encAttrib.length = _attrib.length();
#else
	_rsslPostMsg.msgBase.msgKey.encAttrib = static_cast<const Data&>(attrib).getEncoder().getRsslBuffer();
#endif

	_rsslPostMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_ATTRIB;
	_rsslPostMsg.flags |= RSSL_PSMF_HAS_MSG_KEY;
}

void PostMsgEncoder::serviceId( UInt16 serviceId )
{
	if ( hasServiceName() )
	{
		EmaString text( "Attempt to set serviceId while service name is already set." );
		throwIueException( text );
		return;
	}

	acquireEncIterator();

	_rsslPostMsg.msgBase.msgKey.serviceId = serviceId;
	_rsslPostMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
	_rsslPostMsg.flags |= RSSL_PSMF_HAS_MSG_KEY;
}

bool PostMsgEncoder::hasServiceId() const
{
	return ( ( _rsslPostMsg.flags & RSSL_PSMF_HAS_MSG_KEY ) &&
		( _rsslPostMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ) ? true : false;
}

void PostMsgEncoder::domainType( UInt8 domainType )
{
	acquireEncIterator();

	_rsslPostMsg.msgBase.domainType = domainType;
}

void PostMsgEncoder::streamId( Int32 streamId )
{
	acquireEncIterator();

	_rsslPostMsg.msgBase.streamId = streamId;
}

void PostMsgEncoder::payload( const ComplexType& load )
{
	acquireEncIterator();

	_rsslPostMsg.msgBase.containerType = convertDataType( load.getDataType() );

#ifdef __EMA_COPY_ON_SET__
	RsslBuffer& rsslBuf = static_cast<const Data&>(load).getEncoder().getRsslBuffer();
	_payload.setFrom( rsslBuf.data, rsslBuf.length );

	_rsslPostMsg.msgBase.encDataBody.data = (char*)_payload.c_buf();
	_rsslPostMsg.msgBase.encDataBody.length = _payload.length();
#else
	_rsslPostMsg.msgBase.encDataBody = static_cast<const Data&>(load).getEncoder().getRsslBuffer();
#endif
}

RsslMsg* PostMsgEncoder::getRsslMsg() const
{
	return (RsslMsg*) &_rsslPostMsg;
}
