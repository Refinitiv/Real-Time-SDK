/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "FilterListEncoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "Decoder.h"
#include "FilterList.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

FilterListEncoder::FilterListEncoder() :
 _rsslFilterList(),
 _rsslFilterEntry(),
 _emaLoadType( DataType::NoDataEnum ),
 _containerInitialized( false )
{
}

FilterListEncoder::~FilterListEncoder()
{
}

void FilterListEncoder::clear()
{
	Encoder::clearEncIterator();

	rsslClearFilterList( &_rsslFilterList );
	rsslClearFilterEntry( &_rsslFilterEntry );

	_emaLoadType = DataType::NoDataEnum;

	_containerInitialized = false;
}

void FilterListEncoder::release()
{
	Encoder::releaseEncIterator();

	rsslClearFilterList(&_rsslFilterList);
	rsslClearFilterEntry(&_rsslFilterEntry);

	_emaLoadType = DataType::NoDataEnum;

	_containerInitialized = false;
}

void FilterListEncoder::initEncode( UInt8 dataType )
{
	_rsslFilterList.containerType = dataType;

	RsslRet retCode = rsslEncodeFilterListInit( &(_pEncodeIter->_rsslEncIter), &_rsslFilterList );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		retCode = rsslEncodeFilterListComplete( &(_pEncodeIter->_rsslEncIter), RSSL_FALSE );

		_pEncodeIter->reallocate();

		retCode = rsslEncodeFilterListInit( &(_pEncodeIter->_rsslEncIter), &_rsslFilterList );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to initialize FilterList encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}

	_containerInitialized = true;
}

void FilterListEncoder::addEncodedEntry( UInt8 id, UInt8 action, UInt8 dataType, const EmaBuffer& permission, 
	const char* methodName, const RsslBuffer& rsslBuffer )
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_FILTER_LIST ||
		_levelInfo->_encodingState != RSSL_EIS_ENTRIES)
	{
		EmaString temp("Attemp to add FilterListEntry while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	_rsslFilterEntry.flags = RSSL_FTEF_NONE;

	_rsslFilterEntry.encData = rsslBuffer;
	_rsslFilterEntry.containerType = dataType;
	_rsslFilterEntry.id = id;
	_rsslFilterEntry.action = action;

	rsslFilterEntryApplyHasContainerType( &_rsslFilterEntry );

	if ( permission.length() > 0 )
	{
		rsslFilterEntryApplyHasPermData ( &_rsslFilterEntry );
		_rsslFilterEntry.permData.length = permission.length();
		_rsslFilterEntry.permData.data = const_cast<char *>(permission.c_buf());
	}

	RsslRet retCode = rsslEncodeFilterEntry( &_pEncodeIter->_rsslEncIter, &_rsslFilterEntry );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeFilterEntry( &_pEncodeIter->_rsslEncIter, &_rsslFilterEntry );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to " );
		temp.append( methodName ).append( " while encoding FilterList. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void FilterListEncoder::startEncodingEntry( UInt8 id, UInt8 action, UInt8 dataType, const EmaBuffer& permission, 
	const char* methodName )
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_FILTER_LIST ||
		_levelInfo->_encodingState != RSSL_EIS_ENTRIES)
	{
		EmaString temp("Attemp to add FilterListEntry while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	_rsslFilterEntry.encData.data = 0;
	_rsslFilterEntry.encData.length = 0;
	_rsslFilterEntry.flags = RSSL_FTEF_NONE;

	_rsslFilterEntry.containerType = dataType;
	_rsslFilterEntry.id = id;
	_rsslFilterEntry.action = action;

	rsslFilterEntryApplyHasContainerType( &_rsslFilterEntry );

	if ( permission.length() > 0 )
	{
		rsslFilterEntryApplyHasPermData ( &_rsslFilterEntry );
		_rsslFilterEntry.permData.length = permission.length();
		_rsslFilterEntry.permData.data = const_cast<char *>(permission.c_buf());
	}

	RsslRet retCode = rsslEncodeFilterEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslFilterEntry, 0 );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeFilterEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslFilterEntry, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to start encoding entry in FilterList::" );
		temp.append( methodName ).append( ". Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void FilterListEncoder::endEncodingEntry() const
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_FILTER_LIST ||
		(_levelInfo->_encodingState != RSSL_EIS_ENTRY_INIT &&
		 _levelInfo->_encodingState != RSSL_EIS_WAIT_COMPLETE))
	{
		/*If an internal container is not completed. Internal container empty.*/
		EmaString temp("Attemp to complete FilterList while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	RsslRet retCode = rsslEncodeFilterEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	/* Reallocate does not need here. The data is placed in already allocated memory */

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to end encoding entry in FilterList. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void FilterListEncoder::add( UInt8 filterId, FilterEntry::FilterAction action, const ComplexType& complexType, const EmaBuffer& permission )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	const Encoder& enc = complexType.getEncoder();

	_emaLoadType = complexType.getDataType();

	UInt8 rsslLoadType = enc.convertDataType( _emaLoadType );

	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode(rsslLoadType);
	}

	if ( action == FilterEntry::ClearEnum )
	{
		RsslBuffer rsslBuffer;
		rsslClearBuffer( &rsslBuffer );
		addEncodedEntry( filterId, action, rsslLoadType, permission, "add()", rsslBuffer );
	}
	else if ( complexType.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry(filterId, action, rsslLoadType, permission, "add()", enc.getRsslBuffer());
		else
		{
			EmaString temp( "Attempt to add() a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}
	}
	else if ( complexType.hasDecoder() )
	{
		addEncodedEntry( filterId, action, rsslLoadType, permission, "add()", const_cast<ComplexType&>( complexType ).getDecoder().getRsslBuffer() );
	}
	else
	{
		if ( rsslLoadType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( filterId, action, rsslLoadType, permission, "add()" );
	}
}

void FilterListEncoder::add( UInt8 filterId, FilterEntry::FilterAction action, const EmaBuffer& permission )
{
	if (_containerComplete)
	{
		EmaString temp("Attempt to add an entry after complete() was called.");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	UInt8 rsslLoadType = RSSL_DT_NO_DATA;

	if (!hasEncIterator())
	{
		acquireEncIterator();

		initEncode(rsslLoadType);
	}

	RsslBuffer rsslBuffer;
	rsslClearBuffer(&rsslBuffer);
	addEncodedEntry(filterId, action, rsslLoadType, permission, "add()", rsslBuffer);
}

void FilterListEncoder::complete()
{
	if ( _containerComplete ) return;

	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_NO_DATA );
	}

	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_FILTER_LIST ||
		(_levelInfo->_encodingState != RSSL_EIS_ENTRIES &&
		 _levelInfo->_encodingState != RSSL_EIS_WAIT_COMPLETE))
	{
		/*If an internal container is not completed. Internal container empty.*/
		EmaString temp("Attemp to complete FilterList while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	RsslRet retCode = rsslEncodeFilterListComplete( &(_pEncodeIter->_rsslEncIter), RSSL_TRUE );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to complete FilterList encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
		return;
	}

	_pEncodeIter->setEncodedLength( rsslGetEncodedBufferLength( &(_pEncodeIter->_rsslEncIter) ) );

	if ( !ownsIterator() && _iteratorOwner )
		_iteratorOwner->endEncodingEntry();

	_containerComplete = true;
}

void FilterListEncoder::totalCountHint( UInt8 totalCountHint )
{
	if ( !_containerInitialized )
	{
		rsslFilterListApplyHasTotalCountHint( &_rsslFilterList );
		_rsslFilterList.totalCountHint = totalCountHint;
	}
	else
	{
		EmaString temp( "Invalid attempt to call totalCountHint() when container is initialized." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}
