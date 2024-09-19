/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "VectorEncoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "Decoder.h"
#include "Vector.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

VectorEncoder::VectorEncoder() :
 _rsslVector(),
 _rsslVectorEntry(),
 _emaLoadType( DataType::NoDataEnum ),
 _containerInitialized( false )
{
}

VectorEncoder::~VectorEncoder()
{
}

void VectorEncoder::clear()
{
	Encoder::clearEncIterator();

	rsslClearVector( &_rsslVector );
	rsslClearVectorEntry( &_rsslVectorEntry );

	_emaLoadType = DataType::NoDataEnum;

	_containerInitialized = false;
}

void VectorEncoder::release()
{
	Encoder::releaseEncIterator();

	rsslClearVector(&_rsslVector);
	rsslClearVectorEntry(&_rsslVectorEntry);

	_emaLoadType = DataType::NoDataEnum;

	_containerInitialized = false;
}

void VectorEncoder::initEncode( UInt8 rsslDataType, DataType::DataTypeEnum emaDataType )
{
	if ( !_rsslVector.containerType )
	{
		_rsslVector.containerType = rsslDataType;
		_emaLoadType = emaDataType;
	}
	else if ( _rsslVector.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a DataType different than summaryData's DataType. Passed in ComplexType has DataType of " );
		temp += DataType( emaDataType ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType (_emaLoadType );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	RsslRet retCode = rsslEncodeVectorInit( &(_pEncodeIter->_rsslEncIter), &_rsslVector, 0, 0 );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		retCode = rsslEncodeVectorComplete( &(_pEncodeIter->_rsslEncIter), RSSL_FALSE );

		_pEncodeIter->reallocate();

		retCode = rsslEncodeVectorInit( &(_pEncodeIter->_rsslEncIter), &_rsslVector, 0, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to initialize Vector encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}

	_containerInitialized = true;
}

void VectorEncoder::addEncodedEntry( UInt32 position, UInt8 action, const EmaBuffer& permission, const char* methodName, const RsslBuffer& rsslBuffer )
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_VECTOR ||
		_levelInfo->_encodingState != RSSL_EIS_ENTRIES)
	{
		EmaString temp("Attemp to add VectorEntry while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	_rsslVectorEntry.flags = RSSL_VTEF_NONE;

	_rsslVectorEntry.encData = rsslBuffer;
	_rsslVectorEntry.index = position;
	_rsslVectorEntry.action = action;

	if ( permission.length() > 0 )
	{
		rsslVectorEntryApplyHasPermData ( &_rsslVectorEntry );
		_rsslVectorEntry.permData.length = permission.length();
		_rsslVectorEntry.permData.data = const_cast<char *>(permission.c_buf());
	}

	RsslRet retCode = rsslEncodeVectorEntry( &_pEncodeIter->_rsslEncIter, &_rsslVectorEntry );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeVectorEntry( &_pEncodeIter->_rsslEncIter, &_rsslVectorEntry );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to " );
		temp.append( methodName ).append( " while encoding Vector. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void VectorEncoder::startEncodingEntry( UInt32 position, UInt8 action, const EmaBuffer& permission, const char* methodName )
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_VECTOR ||
		_levelInfo->_encodingState != RSSL_EIS_ENTRIES)
	{
		EmaString temp("Attemp to add VectorEntry while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	_rsslVectorEntry.encData.data = 0;
	_rsslVectorEntry.encData.length = 0;
	_rsslVectorEntry.flags = RSSL_VTEF_NONE;

	_rsslVectorEntry.index = position;
	_rsslVectorEntry.action = action;

	if ( permission.length() > 0 )
	{
		rsslVectorEntryApplyHasPermData ( &_rsslVectorEntry );
		_rsslVectorEntry.permData.length = permission.length();
		_rsslVectorEntry.permData.data = const_cast<char *>(permission.c_buf());
	}

	RsslRet retCode = rsslEncodeVectorEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslVectorEntry, 0 );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeVectorEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslVectorEntry, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to start encoding entry in Vector::" );
		temp.append( methodName ).append( ". Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void VectorEncoder::endEncodingEntry() const
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_VECTOR ||
		(_levelInfo->_encodingState != RSSL_EIS_ENTRY_INIT &&
		 _levelInfo->_encodingState != RSSL_EIS_WAIT_COMPLETE))
	{
		/*If an internal container is not completed. Internal container empty.*/
		EmaString temp("Attemp to complete Vector while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	RsslRet retCode = rsslEncodeVectorEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	/* Reallocate does not need here. The data is placed in already allocated memory */

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to end encoding entry in Vector. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void VectorEncoder::add( UInt32 position, VectorEntry::VectorAction action, const ComplexType& complexType, const EmaBuffer& permission )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	const Encoder& enc = complexType.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( complexType.getDataType() );

	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( rsslDataType, complexType.getDataType() );
	}
	else if ( _rsslVector.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( complexType.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	if ( action == VectorEntry::DeleteEnum || action == VectorEntry::ClearEnum )
	{
		RsslBuffer rsslBuffer;
		rsslClearBuffer( &rsslBuffer );
		addEncodedEntry( position, action, permission, "add()", rsslBuffer );
	}
	else if ( complexType.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( position, action, permission, "add()", enc.getRsslBuffer() );
		else
		{
			EmaString temp( "Attempt to add() a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}
	}
	else if ( complexType.hasDecoder() )
	{
		addEncodedEntry( position, action, permission, "add()", const_cast<ComplexType&>( complexType ).getDecoder().getRsslBuffer() );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( position, action, permission, "add()" );
	}
}

void VectorEncoder::add(UInt32 position, VectorEntry::VectorAction action, const EmaBuffer& permission)
{
	if (_containerComplete)
	{
		EmaString temp("Attempt to add an entry after complete() was called.");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	UInt8 rsslDataType = RSSL_DT_NO_DATA;

	if (!hasEncIterator())
	{
		acquireEncIterator();

		initEncode(rsslDataType, DataType::NoDataEnum);
	}
	else if (_rsslVector.containerType != rsslDataType)
	{
		EmaString temp("Attempt to add an entry with a different DataType. Encode DataType as ");
		temp += DataType(DataType::NoDataEnum).toString();
		temp += EmaString(" while the expected DataType is ");
		temp += DataType(_emaLoadType);
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	RsslBuffer rsslBuffer;
	rsslClearBuffer(&rsslBuffer);
	addEncodedEntry(position, action, permission, "add()", rsslBuffer);
}

void VectorEncoder::complete()
{
	if ( _containerComplete ) return;

	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( convertDataType(_emaLoadType), _emaLoadType);
	}

	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_VECTOR||
		(_levelInfo->_encodingState != RSSL_EIS_ENTRIES &&
		 _levelInfo->_encodingState != RSSL_EIS_SET_DEFINITIONS &&
		 _levelInfo->_encodingState != RSSL_EIS_WAIT_COMPLETE))
	{
		/*If an internal container is not completed. Internal container empty.*/
		EmaString temp("Attemp to complete Vector while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	RsslRet retCode = rsslEncodeVectorComplete( &(_pEncodeIter->_rsslEncIter), RSSL_TRUE );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to complete Vector encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
		return;
	}

	_pEncodeIter->setEncodedLength( rsslGetEncodedBufferLength( &(_pEncodeIter->_rsslEncIter) ) );

	if ( !ownsIterator() && _iteratorOwner )
		_iteratorOwner->endEncodingEntry();
	
	_containerComplete = true;
}

void VectorEncoder::totalCountHint( UInt32 totalCountHint )
{
	if ( !_containerInitialized )
	{
		rsslVectorApplyHasTotalCountHint( &_rsslVector );
		_rsslVector.totalCountHint = totalCountHint;
	}
	else
	{
		EmaString temp( "Invalid attempt to call totalCountHint() when container is initialized." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void VectorEncoder::summaryData( const ComplexType& data )
{
	if ( !_containerInitialized )
	{
		const Encoder& enc = data.getEncoder();

		if ( data.hasEncoder() && enc.ownsIterator() )
		{
			if ( enc.isComplete() )
			{
				rsslVectorApplyHasSummaryData( &_rsslVector );
				_rsslVector.encSummaryData = enc.getRsslBuffer();
			}
			else
			{
				EmaString temp( "Attempt to set summaryData() with a ComplexType while complete() was not called on this ComplexType." );
				throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return;
			}
		}
		else if ( data.hasDecoder() )
		{
			rsslVectorApplyHasSummaryData( &_rsslVector );
			_rsslVector.encSummaryData = const_cast<ComplexType&>(data).getDecoder().getRsslBuffer();
		}
		else
		{
			EmaString temp( "Attempt to pass an empty ComplexType to summaryData() while it is not supported." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		_emaLoadType = data.getDataType();
		_rsslVector.containerType = enc.convertDataType( _emaLoadType );
	}
	else
	{
		EmaString temp( "Invalid attempt to call summaryData() when container is initialized." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void VectorEncoder::sortable( bool sortable )
{
	if ( !_containerInitialized )
	{
		if ( sortable )
			_rsslVector.flags |= RSSL_VTF_SUPPORTS_SORTING;
		else
			_rsslVector.flags &= ~RSSL_VTF_SUPPORTS_SORTING;
	}
	else
	{
		EmaString temp( "Invalid attempt to call sortable() when container is initialized." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}
