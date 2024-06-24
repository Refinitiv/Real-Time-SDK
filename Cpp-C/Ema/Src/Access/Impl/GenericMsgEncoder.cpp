/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "GenericMsgEncoder.h"
#include "ComplexType.h"
#include "Decoder.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

GenericMsgEncoder::GenericMsgEncoder() :
 MsgEncoder()
#ifdef __EMA_COPY_ON_SET__
 ,_permissionData()
#endif
{
	rsslClearGenericMsg( &_rsslGenericMsg );
	_rsslGenericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslGenericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

GenericMsgEncoder::~GenericMsgEncoder()
{
}

void GenericMsgEncoder::clear()
{
	MsgEncoder::clear();

	rsslClearGenericMsg( &_rsslGenericMsg );
	_rsslGenericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslGenericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

void GenericMsgEncoder::extendedHeader( const EmaBuffer& extHeader )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_extendedHeader = extHeader;
	_rsslGenericMsg.extendedHeader.data = (char*)_extendedHeader.c_buf();
	_rsslGenericMsg.extendedHeader.length = _extendedHeader.length();
#else
	_rsslGenericMsg.extendedHeader.data = (char*)extHeader.c_buf();
	_rsslGenericMsg.extendedHeader.length = extHeader.length();
#endif

	rsslGenericMsgApplyHasExtendedHdr( &_rsslGenericMsg );
}

void GenericMsgEncoder::permissionData( const EmaBuffer& permData )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_permissionData = permData;
	_rsslGenericMsg.permData.data = (char*)_permissionData.c_buf();
	_rsslGenericMsg.permData.length = _permissionData.length();
#else
	_rsslGenericMsg.permData.data = (char*)permData.c_buf();
	_rsslGenericMsg.permData.length = permData.length();
#endif

	rsslGenericMsgApplyHasPermData( &_rsslGenericMsg );
}

void GenericMsgEncoder::seqNum( UInt32 seqNum )
{
	acquireEncIterator();

	_rsslGenericMsg.seqNum = seqNum;
	rsslGenericMsgApplyHasSeqNum( &_rsslGenericMsg );
}

void GenericMsgEncoder::secondarySeqNum( UInt32 seqNum )
{
	acquireEncIterator();

	_rsslGenericMsg.secondarySeqNum = seqNum;
	rsslGenericMsgApplyHasSecondarySeqNum( &_rsslGenericMsg );
}

void GenericMsgEncoder::partNum( UInt16 partNum )
{
	acquireEncIterator();

	_rsslGenericMsg.partNum = partNum;
	rsslGenericMsgApplyHasPartNum( &_rsslGenericMsg );
}

void GenericMsgEncoder::complete( bool complete )
{
	acquireEncIterator();

	if ( complete )
		_rsslGenericMsg.flags |= RSSL_GNMF_MESSAGE_COMPLETE;
	else
		_rsslGenericMsg.flags &= ~RSSL_GNMF_MESSAGE_COMPLETE;
}

RsslGenericMsg* GenericMsgEncoder::getRsslGenericMsg() const
{
	return (RsslGenericMsg*)&_rsslGenericMsg;
}

void GenericMsgEncoder::name( const EmaString& name )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_name = name;
	_rsslGenericMsg.msgBase.msgKey.name.data = (char*)_name.c_str();
	_rsslGenericMsg.msgBase.msgKey.name.length = _name.length();
#else
	_rsslGenericMsg.msgBase.msgKey.name.data = (char*)name.c_str();
	_rsslGenericMsg.msgBase.msgKey.name.length = name.length();
#endif

	_rsslGenericMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	_rsslGenericMsg.flags |= RSSL_GNMF_HAS_MSG_KEY;
}

void GenericMsgEncoder::nameType( UInt8 nameType )
{
	acquireEncIterator();

	_rsslGenericMsg.msgBase.msgKey.nameType = nameType;
	_rsslGenericMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
	_rsslGenericMsg.flags |= RSSL_GNMF_HAS_MSG_KEY;
}

void GenericMsgEncoder::filter( UInt32 filter )
{
	acquireEncIterator();

	_rsslGenericMsg.msgBase.msgKey.filter = filter;
	_rsslGenericMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
	_rsslGenericMsg.flags |= RSSL_GNMF_HAS_MSG_KEY;
}

void GenericMsgEncoder::addFilter( UInt32 filter )
{
	acquireEncIterator();

	_rsslGenericMsg.msgBase.msgKey.filter |= filter;
	_rsslGenericMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
	_rsslGenericMsg.flags |= RSSL_GNMF_HAS_MSG_KEY;
}

void GenericMsgEncoder::identifier( Int32 id )
{
	acquireEncIterator();

	_rsslGenericMsg.msgBase.msgKey.identifier = id;
	_rsslGenericMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
	_rsslGenericMsg.flags |= RSSL_GNMF_HAS_MSG_KEY;
}

void GenericMsgEncoder::attrib( const ComplexType& attrib )
{
	acquireEncIterator();

	_rsslGenericMsg.msgBase.msgKey.attribContainerType = convertDataType( attrib.getDataType() );

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
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	_rsslGenericMsg.msgBase.msgKey.encAttrib.data = (char*)_attrib.c_buf();
	_rsslGenericMsg.msgBase.msgKey.encAttrib.length = _attrib.length();
#else
	if ( attrib.hasEncoder() && attrib.getEncoder().ownsIterator() )
		_rsslGenericMsg.msgBase.msgKey.encAttrib = attrib.getEncoder().getRsslBuffer();
	else if ( attrib.hasDecoder() )
		_rsslGenericMsg.msgBase.msgKey.encAttrib = const_cast<ComplexType&>( attrib ).getDecoder().getRsslBuffer();
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp );
		return;
	}
#endif

	_rsslGenericMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_ATTRIB;
	_rsslGenericMsg.flags |= RSSL_GNMF_HAS_MSG_KEY;
}

void GenericMsgEncoder::serviceId( UInt16 serviceId )
{
	if ( hasServiceName() )
	{
		EmaString text( "Attempt to set serviceId while service name is already set." );
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	acquireEncIterator();

	_rsslGenericMsg.msgBase.msgKey.serviceId = serviceId;
	_rsslGenericMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
	_rsslGenericMsg.flags |= RSSL_GNMF_HAS_MSG_KEY;
}

bool GenericMsgEncoder::hasServiceId() const
{
	return ( ( _rsslGenericMsg.flags & RSSL_GNMF_HAS_MSG_KEY ) &&
		( _rsslGenericMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ) ? true : false;
}

void GenericMsgEncoder::domainType( UInt8 domainType )
{
	acquireEncIterator();

	_rsslGenericMsg.msgBase.domainType = domainType;
}

void GenericMsgEncoder::streamId( Int32 streamId )
{
	acquireEncIterator();

	_rsslGenericMsg.msgBase.streamId = streamId;
}

void GenericMsgEncoder::payload( const ComplexType& load )
{
	acquireEncIterator();

	_rsslGenericMsg.msgBase.containerType = convertDataType( load.getDataType() );

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

	_rsslGenericMsg.msgBase.encDataBody.data = (char*)_payload.c_buf();
	_rsslGenericMsg.msgBase.encDataBody.length = _payload.length();
#else
	if ( load.hasEncoder() && load.getEncoder().ownsIterator() )
		_rsslGenericMsg.msgBase.encDataBody = load.getEncoder().getRsslBuffer();
	else if ( load.hasDecoder() )
		_rsslGenericMsg.msgBase.encDataBody = const_cast<ComplexType&>( load ).getDecoder().getRsslBuffer();
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp );
		return;
	}
#endif
}

void GenericMsgEncoder::providerDriven( bool providerDriven )
{
	acquireEncIterator();

	if (providerDriven)
		_rsslGenericMsg.flags |= RSSL_GNMF_PROVIDER_DRIVEN;
	else
		_rsslGenericMsg.flags &= ~RSSL_GNMF_PROVIDER_DRIVEN;
}

RsslMsg* GenericMsgEncoder::getRsslMsg() const
{
	return (RsslMsg*) &_rsslGenericMsg;
}
