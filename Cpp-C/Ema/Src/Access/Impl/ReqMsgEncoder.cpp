/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ReqMsgEncoder.h"
#include "ReqMsg.h"
#include "ComplexType.h"
#include "Decoder.h"
#include "rtr/rsslArray.h"
#include "rtr/rsslElementList.h"
#include "rtr/rsslIterators.h"
#include "rtr/rsslPrimitiveDecoders.h"

using namespace thomsonreuters::ema::access;

ReqMsgEncoder::ReqMsgEncoder() :
 MsgEncoder()
{
	rsslClearRequestMsg( &_rsslRequestMsg );
	_rsslRequestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslRequestMsg.flags = RSSL_RQMF_STREAMING;
	_rsslRequestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

ReqMsgEncoder::~ReqMsgEncoder()
{
}

void ReqMsgEncoder::clear()
{
	MsgEncoder::clear();

	rsslClearRequestMsg( &_rsslRequestMsg );
	_rsslRequestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslRequestMsg.flags = RSSL_RQMF_STREAMING;
	_rsslRequestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

void ReqMsgEncoder::priority( UInt8 priorityClass, UInt16 priorityCount )
{
	acquireEncIterator();

	_rsslRequestMsg.priorityClass = priorityClass;
	_rsslRequestMsg.priorityCount = priorityCount;
	rsslRequestMsgApplyHasPriority( &_rsslRequestMsg );
}

void ReqMsgEncoder::extendedHeader( const EmaBuffer& extHeader )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_extendedHeader = extHeader;
	_rsslRequestMsg.extendedHeader.data = (char*)_extendedHeader.c_buf();
	_rsslRequestMsg.extendedHeader.length = _extendedHeader.length();
#else
	_rsslRequestMsg.extendedHeader.data = (char*)extHeader.c_buf();
	_rsslRequestMsg.extendedHeader.length = extHeader.length();
#endif

	rsslRequestMsgApplyHasExtendedHdr( &_rsslRequestMsg );
}

void ReqMsgEncoder::qos( UInt32 timeliness, UInt32 rate )
{
	acquireEncIterator();

	rsslClearQos( &_rsslRequestMsg.qos );
	rsslClearQos( &_rsslRequestMsg.worstQos );

	_rsslRequestMsg.qos.dynamic = RSSL_FALSE;
	_rsslRequestMsg.worstQos.dynamic = RSSL_FALSE;
	_rsslRequestMsg.flags |= RSSL_RQMF_HAS_QOS;

	switch ( rate )
	{	
	case ReqMsg::TickByTickEnum :
		_rsslRequestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		break;
	case ReqMsg::JustInTimeConflatedEnum :
		_rsslRequestMsg.qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		break;
	case ReqMsg::BestConflatedRateEnum :
		_rsslRequestMsg.qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		_rsslRequestMsg.qos.rateInfo = 1;
		_rsslRequestMsg.worstQos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		_rsslRequestMsg.flags |= RSSL_RQMF_HAS_WORST_QOS;
		break;
	case ReqMsg::BestRateEnum :
		_rsslRequestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		_rsslRequestMsg.worstQos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		_rsslRequestMsg.flags |= RSSL_RQMF_HAS_WORST_QOS;
		break;
	default :
		if ( rate <= 65535 )
		{
			_rsslRequestMsg.qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
			_rsslRequestMsg.qos.rateInfo = rate;
		}
		else
		{
			_rsslRequestMsg.qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		}
		break;
	}

	switch ( timeliness )
	{
	case ReqMsg::RealTimeEnum :
		_rsslRequestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		break;
	case ReqMsg::BestDelayedTimelinessEnum :
		_rsslRequestMsg.qos.timeliness = RSSL_QOS_TIME_DELAYED;
		_rsslRequestMsg.qos.timeInfo = 1;
		_rsslRequestMsg.worstQos.timeliness = RSSL_QOS_TIME_DELAYED;
		_rsslRequestMsg.worstQos.timeInfo = 65535;
		_rsslRequestMsg.flags |= RSSL_RQMF_HAS_WORST_QOS;
		break;
	case ReqMsg::BestTimelinessEnum :
		_rsslRequestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		_rsslRequestMsg.worstQos.timeliness = RSSL_QOS_TIME_DELAYED;
		_rsslRequestMsg.worstQos.timeInfo = 65535;
		_rsslRequestMsg.flags |= RSSL_RQMF_HAS_WORST_QOS;
		break;
	default :
		if ( timeliness <= 65535 )
		{
			_rsslRequestMsg.qos.timeliness = RSSL_QOS_TIME_DELAYED;
			_rsslRequestMsg.qos.timeInfo = timeliness;
		}
		else
		{
			_rsslRequestMsg.qos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		}
	}
}

void ReqMsgEncoder::initialImage( bool initialImage )
{
	acquireEncIterator();

	if ( initialImage )
		_rsslRequestMsg.flags &= ~RSSL_RQMF_NO_REFRESH;
	else
		_rsslRequestMsg.flags |= RSSL_RQMF_NO_REFRESH;
}

void ReqMsgEncoder::interestAfterRefresh( bool interestAfterRefresh )
{
	acquireEncIterator();

	if ( interestAfterRefresh )
		_rsslRequestMsg.flags |= RSSL_RQMF_STREAMING;
	else
		_rsslRequestMsg.flags &= ~RSSL_RQMF_STREAMING;
}

void ReqMsgEncoder::pause( bool pause )
{
	acquireEncIterator();

	if ( pause )
		_rsslRequestMsg.flags |= RSSL_RQMF_PAUSE;
	else
		_rsslRequestMsg.flags &= ~RSSL_RQMF_PAUSE;
}

void ReqMsgEncoder::conflatedInUpdates( bool conflatedInUpdates )
{
	acquireEncIterator();

	if ( conflatedInUpdates )
		_rsslRequestMsg.flags |= RSSL_RQMF_CONF_INFO_IN_UPDATES;
	else
		_rsslRequestMsg.flags &= ~RSSL_RQMF_CONF_INFO_IN_UPDATES;
}

void ReqMsgEncoder::privateStream( bool privateStream )
{
	acquireEncIterator();

	if ( privateStream )
		_rsslRequestMsg.flags |= RSSL_RQMF_PRIVATE_STREAM;
	else
		_rsslRequestMsg.flags &= ~RSSL_RQMF_PRIVATE_STREAM;
}

bool ReqMsgEncoder::getPrivateStream() const
{
	return ( _rsslRequestMsg.flags & RSSL_RQMF_PRIVATE_STREAM ) ? true : false;
}

RsslRequestMsg* ReqMsgEncoder::getRsslRequestMsg() const
{
	return (RsslRequestMsg*)&_rsslRequestMsg;
}

void ReqMsgEncoder::name( const EmaString& name )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_name = name;
	_rsslRequestMsg.msgBase.msgKey.name.data = (char*)_name.c_str();
	_rsslRequestMsg.msgBase.msgKey.name.length = _name.length();
#else
	_rsslRequestMsg.msgBase.msgKey.name.data = (char*)name.c_str();
	_rsslRequestMsg.msgBase.msgKey.name.length = name.length();
#endif

	_rsslRequestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
}

void ReqMsgEncoder::nameType( UInt8 nameType )
{
	acquireEncIterator();

	_rsslRequestMsg.msgBase.msgKey.nameType = nameType;
	_rsslRequestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
}

void ReqMsgEncoder::filter( UInt32 filter )
{
	acquireEncIterator();

	_rsslRequestMsg.msgBase.msgKey.filter = filter;
	_rsslRequestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
}

void ReqMsgEncoder::addFilter( UInt32 filter )
{
	acquireEncIterator();

	_rsslRequestMsg.msgBase.msgKey.filter |= filter;
	_rsslRequestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
}

void ReqMsgEncoder::identifier( Int32 id )
{
	acquireEncIterator();

	_rsslRequestMsg.msgBase.msgKey.identifier = id;
	_rsslRequestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
}

void ReqMsgEncoder::attrib( const ComplexType& attrib )
{
	acquireEncIterator();

	_rsslRequestMsg.msgBase.msgKey.attribContainerType = convertDataType( attrib.getDataType() );

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
		throwIueException( temp );
		return;
	}

	_rsslRequestMsg.msgBase.msgKey.encAttrib.data = (char*)_attrib.c_buf();
	_rsslRequestMsg.msgBase.msgKey.encAttrib.length = _attrib.length();
#else
	if ( attrib.hasEncoder() && attrib.getEncoder().ownsIterator() )
		_rsslRequestMsg.msgBase.msgKey.encAttrib = attrib.getEncoder().getRsslBuffer();
	else if ( attrib.hasDecoder() )
		_rsslRequestMsg.msgBase.msgKey.encAttrib = const_cast<ComplexType&>( attrib ).getDecoder().getRsslBuffer();
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp );
		return;
	}
#endif

	_rsslRequestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_ATTRIB;
}

void ReqMsgEncoder::serviceId( UInt16 serviceId )
{
	if ( hasServiceName() )
	{
		EmaString text( "Attempt to set serviceId while service name is already set." );
		throwIueException( text );
		return;
	}

	acquireEncIterator();

	_rsslRequestMsg.msgBase.msgKey.serviceId = serviceId;
	_rsslRequestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
}

bool ReqMsgEncoder::hasServiceId() const
{
	return ( _rsslRequestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ? true : false;
}

void ReqMsgEncoder::domainType( UInt8 domainType )
{
	acquireEncIterator();

	_rsslRequestMsg.msgBase.domainType = domainType;
}

void ReqMsgEncoder::streamId( Int32 streamId )
{
	acquireEncIterator();

	_rsslRequestMsg.msgBase.streamId = streamId;
}

void ReqMsgEncoder::payload( const ComplexType& load )
{
	acquireEncIterator();

	_rsslRequestMsg.msgBase.containerType = convertDataType( load.getDataType() );

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
		throwIueException( temp );
		return;
	}

	_rsslRequestMsg.msgBase.encDataBody.data = (char*)_payload.c_buf();
	_rsslRequestMsg.msgBase.encDataBody.length = _payload.length();
#else
	if ( load.hasEncoder() && load.getEncoder().ownsIterator() )
		_rsslRequestMsg.msgBase.encDataBody = load.getEncoder().getRsslBuffer();
	else if ( load.hasDecoder() )
		_rsslRequestMsg.msgBase.encDataBody = const_cast<ComplexType&>( load ).getDecoder().getRsslBuffer();
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp );
		return;
	}
#endif

	if ( _rsslRequestMsg.msgBase.containerType == RSSL_DT_ELEMENT_LIST )
		checkBatchView( &_rsslRequestMsg.msgBase.encDataBody );
}

RsslMsg* ReqMsgEncoder::getRsslMsg() const
{
	return (RsslMsg*) &_rsslRequestMsg;
}

void ReqMsgEncoder::checkBatchView( RsslBuffer* pRsslBuffer )
{
	RsslElementList	rsslElementList;
	RsslElementEntry rsslElementEntry;
	RsslDecodeIterator decodeIter;

	rsslClearDecodeIterator( &decodeIter );
	rsslClearElementEntry( &rsslElementEntry );
	rsslClearElementList( &rsslElementList );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &decodeIter, pRsslBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "ReqMsgEncoder::checkBatchView(): Failed to set iterator buffer in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp );
		return;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "ReqMsgEncoder::checkBatchView(): Failed to set iterator version in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp );
		return;
	}

	retCode = rsslDecodeElementList( &decodeIter, &rsslElementList, 0 );

	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString temp( "ReqMsgEncoder::checkBatchView(): Failed to decode ElementList in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp );
		return;
	}

	while ( true )
	{
		retCode = rsslDecodeElementEntry( &decodeIter, &rsslElementEntry );

		switch ( retCode )
		{
		case RSSL_RET_END_OF_CONTAINER :
				return;

		case RSSL_RET_SUCCESS :

			if ( rsslBufferIsEqual( &rsslElementEntry.name, &RSSL_ENAME_VIEW_DATA ) && rsslElementEntry.dataType == RSSL_DT_ARRAY )
				_rsslRequestMsg.flags |= RSSL_RQMF_HAS_VIEW;

			else if ( rsslBufferIsEqual( &rsslElementEntry.name, &RSSL_ENAME_BATCH_ITEM_LIST ) && rsslElementEntry.dataType == RSSL_DT_ARRAY )
			{
				RsslArray rsslArray;
				rsslClearArray( &rsslArray );
				rsslArray.encData = rsslElementEntry.encData;

				if ( rsslDecodeArray( &decodeIter, &rsslArray ) >= RSSL_RET_SUCCESS )
				{
					if ( rsslArray.primitiveType == RSSL_DT_ASCII_STRING )
					{
						RsslBuffer rsslBuffer;

						while ( rsslDecodeArrayEntry( &decodeIter, &rsslBuffer ) != RSSL_RET_END_OF_CONTAINER )
							_rsslRequestMsg.flags |= RSSL_RQMF_HAS_BATCH;
					}
				}
			}
			break;

		case RSSL_RET_INCOMPLETE_DATA :
		case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		default :
			{
				EmaString temp( "ReqMsgEncoder::checkBatchView(): Failed to decode ElementEntry. Internal error " );
				temp.append( rsslRetCodeToString( retCode ) );
				throwIueException( temp );
			}
			return;
		}
	}
}

UInt32 ReqMsgEncoder::getBatchItemListSize() const
{
	if ( !( _rsslRequestMsg.flags & RSSL_RQMF_HAS_BATCH ) ) return 0;

	RsslElementList	rsslElementList;
	RsslElementEntry rsslElementEntry;
	RsslDecodeIterator decodeIter;

	rsslClearDecodeIterator( &decodeIter );
	rsslClearElementEntry( &rsslElementEntry );
	rsslClearElementList( &rsslElementList );

	RsslBuffer tempRsslBuffer = _rsslRequestMsg.msgBase.encDataBody;

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &decodeIter, &tempRsslBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "ReqMsgEncoder::checkBatchView(): Failed to set iterator buffer in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp );
		return 0;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "ReqMsgEncoder::checkBatchView(): Failed to set iterator version in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp );
		return 0;
	}

	retCode = rsslDecodeElementList( &decodeIter, &rsslElementList, 0 );

	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString temp( "ReqMsgEncoder::checkBatchView(): Failed to decode ElementList in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp );
		return 0;
	}

	while ( true )
	{
		retCode = rsslDecodeElementEntry( &decodeIter, &rsslElementEntry );

		switch ( retCode )
		{
		case RSSL_RET_END_OF_CONTAINER :
			return 0;

		case RSSL_RET_SUCCESS :
			{
				UInt32 batchListSize = 0;

				if ( rsslBufferIsEqual( &rsslElementEntry.name, &RSSL_ENAME_BATCH_ITEM_LIST ) && rsslElementEntry.dataType == RSSL_DT_ARRAY )
				{
					RsslArray rsslArray;
					rsslClearArray( &rsslArray );
					rsslArray.encData = rsslElementEntry.encData;

					if ( rsslDecodeArray( &decodeIter, &rsslArray ) >= RSSL_RET_SUCCESS )
					{
						if ( rsslArray.primitiveType == RSSL_DT_ASCII_STRING )
						{
							RsslBuffer rsslBuffer;
							while ( rsslDecodeArrayEntry( &decodeIter, &rsslBuffer ) != RSSL_RET_END_OF_CONTAINER )
								batchListSize++;
						}
					}

					return batchListSize;
				}
			}
			break;

		case RSSL_RET_INCOMPLETE_DATA :
		case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		default :
			{
				EmaString temp( "ReqMsgEncoder::checkBatchView(): Failed to decode ElementEntry. Internal error " );
				temp.append( rsslRetCodeToString( retCode ) );
				throwIueException( temp );
			}
			return 0;
		}
	}

	return 0;
}
