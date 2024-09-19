/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "AckMsgEncoder.h"
#include "ComplexType.h"
#include "Decoder.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

AckMsgEncoder::AckMsgEncoder() :
	MsgEncoder()
#ifdef __EMA_COPY_ON_SET__
	, _text()
#endif
{
	rsslClearAckMsg( &_rsslAckMsg );
	_rsslAckMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslAckMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

AckMsgEncoder::~AckMsgEncoder()
{
}

void AckMsgEncoder::clear()
{
	MsgEncoder::clear();

	rsslClearAckMsg( &_rsslAckMsg );
	_rsslAckMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslAckMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

void AckMsgEncoder::release()
{
	MsgEncoder::release();

	rsslClearAckMsg(&_rsslAckMsg);
	_rsslAckMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslAckMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

void AckMsgEncoder::extendedHeader( const EmaBuffer& extHeader )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_extendedHeader = extHeader;
	_rsslAckMsg.extendedHeader.data = ( char* )_extendedHeader.c_buf();
	_rsslAckMsg.extendedHeader.length = _extendedHeader.length();
#else
	_rsslAckMsg.extendedHeader.data = ( char* )extHeader.c_buf();
	_rsslAckMsg.extendedHeader.length = extHeader.length();
#endif

	rsslAckMsgApplyHasExtendedHdr( &_rsslAckMsg );
}

void AckMsgEncoder::ackId( UInt32 ackId )
{
	acquireEncIterator();

	_rsslAckMsg.ackId = ackId;
}

void AckMsgEncoder::seqNum( UInt32 seqNum )
{
	acquireEncIterator();

	_rsslAckMsg.seqNum = seqNum;
	rsslAckMsgApplyHasSeqNum( &_rsslAckMsg );
}

RsslAckMsg* AckMsgEncoder::getRsslAckMsg() const
{
	return ( RsslAckMsg* )&_rsslAckMsg;
}

void AckMsgEncoder::nackCode( UInt8 nackCode )
{
	_rsslAckMsg.nakCode = nackCode;
	_rsslAckMsg.flags |= RSSL_AKMF_HAS_NAK_CODE;
}

void AckMsgEncoder::text( const EmaString& text )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_text = text;
	_rsslAckMsg.text.data = ( char* )_text.c_str();
	_rsslAckMsg.text.length = _text.length();
#else
	_rsslAckMsg.text.data = ( char* )text.c_str();
	_rsslAckMsg.text.length = text.length();
#endif

	rsslAckMsgApplyHasText( &_rsslAckMsg );
}

void AckMsgEncoder::privateStream( bool privateStream )
{
	acquireEncIterator();

	if ( privateStream )
		_rsslAckMsg.flags |= RSSL_AKMF_PRIVATE_STREAM;
	else
		_rsslAckMsg.flags &= ~RSSL_AKMF_PRIVATE_STREAM;
}

void AckMsgEncoder::name( const EmaString& name )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_name = name;
	_rsslAckMsg.msgBase.msgKey.name.data = ( char* )_name.c_str();
	_rsslAckMsg.msgBase.msgKey.name.length = _name.length();
#else
	_rsslAckMsg.msgBase.msgKey.name.data = ( char* )name.c_str();
	_rsslAckMsg.msgBase.msgKey.name.length = name.length();
#endif

	_rsslAckMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	_rsslAckMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
}

void AckMsgEncoder::nameType( UInt8 nameType )
{
	acquireEncIterator();

	_rsslAckMsg.msgBase.msgKey.nameType = nameType;
	_rsslAckMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
	_rsslAckMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
}

void AckMsgEncoder::filter( UInt32 filter )
{
	acquireEncIterator();

	_rsslAckMsg.msgBase.msgKey.filter = filter;
	_rsslAckMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
	_rsslAckMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
}

void AckMsgEncoder::addFilter( UInt32 filter )
{
	acquireEncIterator();

	_rsslAckMsg.msgBase.msgKey.filter |= filter;
	_rsslAckMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
	_rsslAckMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
}

void AckMsgEncoder::identifier( Int32 id )
{
	acquireEncIterator();

	_rsslAckMsg.msgBase.msgKey.identifier = id;
	_rsslAckMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
	_rsslAckMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
}

void AckMsgEncoder::attrib( const ComplexType& attrib )
{
	acquireEncIterator();

	_rsslAckMsg.msgBase.msgKey.attribContainerType = convertDataType( attrib.getDataType() );

#ifdef __EMA_COPY_ON_SET__
	if ( attrib.hasEncoder() && attrib.getEncoder().ownsIterator() )
	{
		const RsslBuffer& rsslBuf = attrib.getEncoder().getRsslBuffer();
		_attrib.setFrom( rsslBuf.data, rsslBuf.length );
	}
	else if ( attrib.hasDecoder() )
	{
		const RsslBuffer& rsslBuf = const_cast<ComplexType&>( attrib ).getDecoder().getRsslBuffer();
		_attrib.setFrom( rsslBuf.data, rsslBuf.length );
	}
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	_rsslAckMsg.msgBase.msgKey.encAttrib.data = ( char* )_attrib.c_buf();
	_rsslAckMsg.msgBase.msgKey.encAttrib.length = _attrib.length();
#else
	if ( attrib.hasEncoder() && attrib.getEncoder().ownsIterator() )
		_rsslAckMsg.msgBase.msgKey.encAttrib = attrib.getEncoder().getRsslBuffer();
	else if ( attrib.hasDecoder() )
		_rsslAckMsg.msgBase.msgKey.encAttrib = const_cast<ComplexType&>( attrib ).getDecoder().getRsslBuffer();
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp );
		return;
	}
#endif

	_rsslAckMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_ATTRIB;
	_rsslAckMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
}

void AckMsgEncoder::serviceId( UInt16 serviceId )
{
	if ( hasServiceName() )
	{
		EmaString text( "Attempt to set serviceId while service name is already set." );
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	acquireEncIterator();

	_rsslAckMsg.msgBase.msgKey.serviceId = serviceId;
	_rsslAckMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
	_rsslAckMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
}

bool AckMsgEncoder::hasServiceId() const
{
	return ( ( _rsslAckMsg.flags & RSSL_AKMF_HAS_MSG_KEY ) &&
	         ( _rsslAckMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ) ? true : false;
}

void AckMsgEncoder::domainType( UInt8 domainType )
{
	acquireEncIterator();

	_rsslAckMsg.msgBase.domainType = domainType;
}

void AckMsgEncoder::streamId( Int32 streamId )
{
	acquireEncIterator();

	_rsslAckMsg.msgBase.streamId = streamId;
}

void AckMsgEncoder::payload( const ComplexType& load )
{
	acquireEncIterator();

	_rsslAckMsg.msgBase.containerType = convertDataType( load.getDataType() );

#ifdef __EMA_COPY_ON_SET__
	if ( load.hasEncoder() && load.getEncoder().ownsIterator() )
	{
		const RsslBuffer& rsslBuf = load.getEncoder().getRsslBuffer();
		_payload.setFrom( rsslBuf.data, rsslBuf.length );
	}
	else if ( load.hasDecoder() )
	{
		const RsslBuffer& rsslBuf = const_cast<ComplexType&>( load ).getDecoder().getRsslBuffer();
		_payload.setFrom( rsslBuf.data, rsslBuf.length );
	}
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	_rsslAckMsg.msgBase.encDataBody.data = ( char* )_payload.c_buf();
	_rsslAckMsg.msgBase.encDataBody.length = _payload.length();
#else
	if ( loadhasEncoder() && load.getEncoder().ownsIterator() )
		_rsslAckMsg.msgBase.encDataBody = load.getEncoder().getRsslBuffer();
	else if ( load.hasDecoder() )
		_rsslAckMsg.msgBase.encDataBody = const_cast<ComplexType&>( load ).getDecoder().getRsslBuffer();
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp );
		return;
	}
#endif
}

RsslMsg* AckMsgEncoder::getRsslMsg() const
{
	return ( RsslMsg* ) &_rsslAckMsg;
}
