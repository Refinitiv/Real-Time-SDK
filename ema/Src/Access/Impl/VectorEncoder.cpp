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
#include "Vector.h"

using namespace thomsonreuters::ema::access;

VectorEncoder::VectorEncoder() :
  _containerInitialized( false ),
  _rsslVector(),
 _rsslVectorEntry()
{
	rsslClearVector( &_rsslVector );
	rsslClearVectorEntry( &_rsslVectorEntry );
}

VectorEncoder::~VectorEncoder()
{
}

void VectorEncoder::clear()
{
	Encoder::releaseEncIterator();

	rsslClearVector( &_rsslVector );
	rsslClearVectorEntry( &_rsslVectorEntry );

	_containerInitialized = false;
}

void VectorEncoder::initEncode( UInt8 dataType )
{
	_rsslVector.containerType = dataType;

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

void VectorEncoder::addEncodedEntry( UInt32 position, UInt8 action, const EmaBuffer& permission, 
	const char* methodName, RsslBuffer& rsslBuffer )
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

void VectorEncoder::startEncodingEntry( UInt32 position, UInt8 action, const EmaBuffer& permission, 
	const char* methodName )
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
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(complexType).getEncoder() ).convertDataType( complexType.getDataType() );

	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( dataType );
	}

	if ( static_cast<const ComplexType&>(complexType).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( position, action, permission, "add()", static_cast<const ComplexType&>(complexType).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(complexType).getEncoder() ) );
		startEncodingEntry( position, action, permission, "add()" );
	}
}

void VectorEncoder::complete()
{
	if ( !hasEncIterator() )
	{
		EmaString temp( "Cannot complete an empty Vector" );
		throwIueException( temp );
		return;
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

void VectorEncoder::summary( const ComplexType& data )
{
	if ( !_containerInitialized )
	{
		if ( static_cast<const ComplexType&>(data).getEncoder().isComplete() )
		{
			rsslVectorApplyHasSummaryData( &_rsslVector );
			_rsslVector.encSummaryData = static_cast<const ComplexType&>(data).getEncoder().getRsslBuffer();
		}
		else
		{
			EmaString temp( "Invalid attempt to pass not completed container to summary()." );
			throwIueException( temp );
		}
	}
	else
	{
		EmaString temp( "Invalid attempt to call summary() when container is not empty." );
		throwIueException( temp );
	}
}
