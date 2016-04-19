/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "VectorEncoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "Decoder.h"
#include "Vector.h"

using namespace thomsonreuters::ema::access;

VectorEncoder::VectorEncoder() :
 _rsslVector(),
 _rsslVectorEntry(),
 _emaDataType( DataType::NoDataEnum ),
 _containerInitialized( false )
{
}

VectorEncoder::~VectorEncoder()
{
}

void VectorEncoder::clear()
{
	Encoder::releaseEncIterator();

	rsslClearVector( &_rsslVector );
	rsslClearVectorEntry( &_rsslVectorEntry );

	_emaDataType = DataType::NoDataEnum;

	_containerInitialized = false;
}

void VectorEncoder::initEncode( UInt8 rsslDataType, DataType::DataTypeEnum emaDataType )
{
	if ( !_rsslVector.containerType )
	{
		_rsslVector.containerType = rsslDataType;
		_emaDataType = emaDataType;
	}
	else if ( _rsslVector.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a DataType different than summaryData's DataType. Passed in ComplexType has DataType of " );
		temp += DataType( emaDataType ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaDataType );
		throwIueException( temp );
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
		throwIueException( temp );
	}
}

void VectorEncoder::addEncodedEntry( UInt32 position, UInt8 action, const EmaBuffer& permission, const char* methodName, const RsslBuffer& rsslBuffer )
{
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
		throwIueException( temp );
	}
}

void VectorEncoder::startEncodingEntry( UInt32 position, UInt8 action, const EmaBuffer& permission, const char* methodName )
{
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
		throwIueException( temp );
	}
}

void VectorEncoder::endEncodingEntry() const
{
	RsslRet retCode = rsslEncodeVectorEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeElementEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to end encoding entry in Vector. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void VectorEncoder::add( UInt32 position, VectorEntry::VectorAction action, const ComplexType& complexType, const EmaBuffer& permission )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
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
		temp += DataType( _emaDataType );
		throwIueException( temp );
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
			throwIueException( temp );
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
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( position, action, permission, "add()" );
	}
}

void VectorEncoder::complete()
{
	if ( _containerComplete ) return;

	if ( !hasEncIterator() )
	{
		if ( _rsslVector.containerType )
		{
			acquireEncIterator();

			initEncode( _rsslVector.containerType, _emaDataType );
		}
		else
		{
			EmaString temp( "Attempt to complete() while no Vector::add() or Vector::summaryData() were called yet." );
			throwIueException( temp );
			return;
		}
	}

	RsslRet retCode = rsslEncodeVectorComplete( &(_pEncodeIter->_rsslEncIter), RSSL_TRUE );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to complete Vector encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
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
		EmaString temp( "Invalid attempt to call totalCountHint() when container is not empty." );
		throwIueException( temp );
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
				throwIueException( temp );
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
			throwIueException( temp );
			return;
		}

		_emaDataType = data.getDataType();
		_rsslVector.containerType = enc.convertDataType( _emaDataType );
	}
	else
	{
		EmaString temp( "Invalid attempt to call summaryData() when container is not empty." );
		throwIueException( temp );
	}
}
