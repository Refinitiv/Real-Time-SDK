/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmError.h"
#include "OmmErrorDecoder.h"
#include "Utilities.h"
#include <new>

using namespace thomsonreuters::ema::access;

const EmaString NoErrorString( "NoError" );
const EmaString NoDictionaryString( "NoDictionary" );
const EmaString IteratorSetFailureString( "IteratorSetFailure" );
const EmaString IteratorOverrunString( "IteratorOverrun" );
const EmaString FieldIdNotFoundString( "FieldIdNotFound" );
const EmaString IncompleteDataString( "IncompleteData" );
const EmaString UnsupportedDataTypeString( "UnsupportedDataType" );
const EmaString NoSetDefinitionString( "NoSetDefinition" );
const EmaString UnknownErrorString( "UnknownError" );
EmaString TempErrorCodeString;


OmmError::OmmError() :
 _toString(),
 _pDecoder( new ( _space ) OmmErrorDecoder() )
{
}

OmmError::~OmmError()
{
	_pDecoder->~OmmErrorDecoder();
}

const EmaString& OmmError::getErrorCodeAsString() const
{
	switch ( getErrorCode() )
	{
	case NoErrorEnum :
		return NoErrorString;
	case NoDictionaryEnum :
		return NoDictionaryString;
	case FieldIdNotFoundEnum :
		return FieldIdNotFoundString;
	case IteratorOverrunEnum :
		return IteratorOverrunString;
	case IteratorSetFailureEnum :
		return IteratorSetFailureString;
	case IncompleteDataEnum :
		return IncompleteDataString;
	case NoSetDefinitionEnum :
		return NoSetDefinitionString;
	case UnsupportedDataTypeEnum :
		return UnsupportedDataTypeString;
	case UnknownErrorEnum :
		return UnknownErrorString;
	default :
		return TempErrorCodeString.set( "Unrecognized ErrorCode value " ).append( (Int64)getErrorCode() );
	}
}

DataType::DataTypeEnum OmmError::getDataType() const
{
	return DataType::ErrorEnum;
}

Data::DataCode OmmError::getCode() const
{
	return Data::NoCodeEnum;
}

OmmError::ErrorCode OmmError::getErrorCode() const
{
	return _pDecoder->getErrorCode();
}

const EmaString& OmmError::toString() const
{
	return toString( 0 );
}

const EmaString& OmmError::toString( UInt64 indent ) const
{
	addIndent( _toString.clear(), indent++ ).append( "OmmError" );

	++indent;
	addIndent( _toString, indent, true ).append( "ErrorCode=\"" ).append( getErrorCodeAsString() ).append( "\"" );
	--indent;

	addIndent( _toString, indent, true ).append( "OmmErrorEnd\n" );

	return _toString;
}

const EmaBuffer& OmmError::getAsHex() const
{
	return _pDecoder->getAsHex();
}

Decoder& OmmError::getDecoder()
{
	return *_pDecoder;
}

const Encoder& OmmError::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}
