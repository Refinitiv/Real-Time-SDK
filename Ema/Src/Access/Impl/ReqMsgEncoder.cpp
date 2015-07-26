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
#include "rtr/rsslArray.h"
#include "rtr/rsslElementList.h"
#include "rtr/rsslIterators.h"
#include "rtr/rsslPrimitiveDecoders.h"

using namespace thomsonreuters::ema::access;

ReqMsgEncoder::ReqMsgEncoder() :
 MsgEncoder(),
 _pBatchItemList( 0 )
{
	rsslClearRequestMsg( &_rsslRequestMsg );
	_rsslRequestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslRequestMsg.flags = RSSL_RQMF_STREAMING;
	_rsslRequestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

ReqMsgEncoder::~ReqMsgEncoder()
{
	if ( _pBatchItemList )
	{
		delete _pBatchItemList;
		_pBatchItemList = 0;
	}
}

void ReqMsgEncoder::clear()
{
	MsgEncoder::clear();

	rsslClearRequestMsg( &_rsslRequestMsg );
	_rsslRequestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslRequestMsg.flags = RSSL_RQMF_STREAMING;
	_rsslRequestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	if ( _pBatchItemList )
		_pBatchItemList->clear();
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
	_attrib.setFrom( static_cast<const Data&>(attrib).getEncoder().getRsslBuffer().data,
					static_cast<const Data&>(attrib).getEncoder().getRsslBuffer().length );

	_rsslRequestMsg.msgBase.msgKey.encAttrib.data = (char*)_attrib.c_buf();
	_rsslRequestMsg.msgBase.msgKey.encAttrib.length = _attrib.length();
#else
	_rsslRequestMsg.msgBase.msgKey.encAttrib = static_cast<const Data&>(attrib).getEncoder().getRsslBuffer();
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
	_payload.setFrom( static_cast<const Data&>(load).getEncoder().getRsslBuffer().data,
					static_cast<const Data&>(load).getEncoder().getRsslBuffer().length );

	_rsslRequestMsg.msgBase.encDataBody.data = (char*)_payload.c_buf();
	_rsslRequestMsg.msgBase.encDataBody.length = _payload.length();
#else
	_rsslRequestMsg.msgBase.encDataBody = static_cast<const Data&>(load).getEncoder().getRsslBuffer();
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
	RsslBuffer rsslBuffer;

	rsslClearDecodeIterator( &decodeIter );
	rsslClearElementEntry( &rsslElementEntry );
	rsslClearElementList( &rsslElementList );
	rsslClearBuffer( &rsslBuffer );

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
			{
				if ( rsslBuffer.length != 0 )
				{
					memcpy(pRsslBuffer->data, rsslBuffer.data, rsslBuffer.length );
					pRsslBuffer->length = rsslBuffer.length;
					delete [] rsslBuffer.data;
				}

				return;
			}
		case RSSL_RET_SUCCESS :
			if ( rsslElementEntry.name.length == 9 &&
				0 == memcmp( rsslElementEntry.name.data, ":ViewData", 9 ) )
			{
				_rsslRequestMsg.flags |= RSSL_RQMF_HAS_VIEW;
			}
			else if ( rsslElementEntry.name.length == 9 &&
				0 == memcmp( rsslElementEntry.name.data, ":ItemList", 9 ) )
			{
				if ( !_pBatchItemList )
					_pBatchItemList = new EmaVector<EmaString>( 10 );
				else
					_pBatchItemList->clear();

				if ( rsslElementEntry.dataType == RSSL_DT_ARRAY )
				{
					RsslArray rsslArray;
					rsslClearArray( &rsslArray );
					rsslArray.encData = rsslElementEntry.encData;

					UInt32 numOfItems = 0;

					if ( rsslDecodeArray( &decodeIter, &rsslArray ) >= RSSL_RET_SUCCESS )
					{
						if ( rsslArray.primitiveType == RSSL_DT_ASCII_STRING )
						{
							RsslBuffer rsslBuffer;

							while ( rsslDecodeArrayEntry( &decodeIter, &rsslBuffer ) != RSSL_RET_END_OF_CONTAINER )
							{
								if ( rsslDecodeBuffer( &decodeIter, &rsslBuffer ) == RSSL_RET_SUCCESS )
								{
									numOfItems++;
									addBatchItemList( rsslBuffer );
								}
							}
						}
					}

					if ( getBatchItemList().size() > 0 )
					{
						_rsslRequestMsg.flags |= RSSL_RQMF_HAS_BATCH;

						if ( numOfItems > getBatchItemList().size() )
						{
							char * buffer = new char[pRsslBuffer->length];
							rsslBuffer.data = buffer;
							rsslBuffer.length = pRsslBuffer->length;
							retCode = reEncodeItemList( pRsslBuffer, getBatchItemList(), &rsslBuffer );

							if ( retCode != RSSL_RET_SUCCESS )
							{
								delete [] rsslBuffer.data;
								rsslClearBuffer( &rsslBuffer );
								EmaString temp( "ReqMsgEncoder::checkBatchView(): Failed to re-encode ElementList in ReqMsg::payload(). Internal error " );
								temp.append( rsslRetCodeToString( retCode ) );
								throwIueException( temp );
								return;
							}
						}
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
			break;
		}
	}
}

void ReqMsgEncoder::addBatchItemList( const RsslBuffer & itemName )
{
	EmaString name(itemName.data,itemName.length);

	if ( 0 > _pBatchItemList->getPositionOf( name ) )
		_pBatchItemList->push_back( name);
}

const EmaVector<EmaString>& ReqMsgEncoder::getBatchItemList() const
{
	return *_pBatchItemList;
}

RsslRet ReqMsgEncoder::reEncodeItemList( RsslBuffer* rsslBuffer, const EmaVector<EmaString>& batchItemList, RsslBuffer* reEncodedBuffer )
{
	RsslDecIterator decodeIter;
	RsslEncodeIterator encodeIter;
	RsslElementList elementList, encodeElementList;
	RsslElementEntry elementEntry, encodeElementEntry;

	rsslClearElementList(&elementList);
	rsslClearElementList(&encodeElementList);

	rsslElementListApplyHasStandardData(&encodeElementList);

	rsslClearDecodeIterator( &decodeIter );
	rsslClearEncodeIterator( &encodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &decodeIter, rsslBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
		return retCode;

	retCode = rsslSetEncodeIteratorBuffer( &encodeIter, reEncodedBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
		return retCode;

	retCode = rsslSetDecodeIteratorRWFVersion( &decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	if ( RSSL_RET_SUCCESS != retCode )
		return retCode;

	retCode = rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	if ( RSSL_RET_SUCCESS != retCode )
		return retCode;

	retCode = rsslDecodeElementList( &decodeIter, &elementList, 0 );
	if ( retCode != RSSL_RET_SUCCESS )
		return retCode;

	retCode = rsslEncodeElementListInit( &encodeIter, &encodeElementList, 0, 0 );
	if ( retCode != RSSL_RET_SUCCESS )
		return retCode;

	while ( true )
	{
		retCode = rsslDecodeElementEntry( &decodeIter, &elementEntry );

		switch ( retCode )
		{
		case RSSL_RET_END_OF_CONTAINER:
			break;
		case RSSL_RET_SUCCESS:

			rsslClearElementEntry( &encodeElementEntry );

			if ( elementEntry.name.length == 9 &&
				0 == memcmp( elementEntry.name.data, ":ItemList", 9 ) )
			{
				RsslArray rsslArray;
				RsslBuffer rsslBuffer;

				encodeElementEntry.dataType = RSSL_DT_ARRAY;
				encodeElementEntry.name.data = elementEntry.name.data;
				encodeElementEntry.name.length = elementEntry.name.length;

				retCode = rsslEncodeElementEntryInit( &encodeIter, &encodeElementEntry, 0 );
				if ( retCode != RSSL_RET_SUCCESS)
					break;

				rsslClearArray( &rsslArray );
				rsslArray.primitiveType = RSSL_DT_ASCII_STRING;

				retCode = rsslEncodeArrayInit( &encodeIter, &rsslArray );
				if ( retCode != RSSL_RET_SUCCESS )
					break;

				for( UInt32 i = 0 ; i < batchItemList.size(); i++ )
				{
					rsslBuffer.data = (char *)batchItemList[i].c_str();
					rsslBuffer.length = batchItemList[i].length();

					retCode = rsslEncodeArrayEntry( &encodeIter, 0, &rsslBuffer );
					if ( retCode != RSSL_RET_SUCCESS)
						break;
				}

				retCode = rsslEncodeArrayComplete( &encodeIter, RSSL_TRUE );
				if ( retCode != RSSL_RET_SUCCESS)
					break;

				retCode = rsslEncodeElementEntryComplete( &encodeIter, RSSL_TRUE );
				if ( retCode != RSSL_RET_SUCCESS )
					break;
			}
			else
			{
				encodeElementEntry.name = elementEntry.name;
				encodeElementEntry.dataType = elementEntry.dataType;
				encodeElementEntry.encData = elementEntry.encData;

				retCode = rsslEncodeElementEntry( &encodeIter, &encodeElementEntry, 0 );
				if ( retCode != RSSL_RET_SUCCESS )
					break;
			}

			break;
		case RSSL_RET_INCOMPLETE_DATA :
		case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		default :
			return retCode;
			break;
		}

		if ( retCode != RSSL_RET_SUCCESS )
			break;
	}

	if ( retCode == RSSL_RET_END_OF_CONTAINER )
	{
		retCode = rsslEncodeElementListComplete( &encodeIter, RSSL_TRUE );
		reEncodedBuffer->length = rsslGetEncodedBufferLength( &encodeIter );
	}

	return retCode;
}
