/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "NoDataImpl.h"
#include "EmaString.h"
#include "Utilities.h"

using namespace thomsonreuters::ema::access;

NoDataImpl::NoDataImpl() :
 _rsslBuffer(),
 _hexBuffer(),
 _majVer( RSSL_RWF_MAJOR_VERSION ),
 _minVer( RSSL_RWF_MINOR_VERSION ),
 _toString(),
 _errorCode( OmmError::NoErrorEnum )
{
}

NoDataImpl::~NoDataImpl()
{
}

DataType::DataTypeEnum NoDataImpl::getDataType() const
{
	return DataType::NoDataEnum;
}

const EmaString& NoDataImpl::toString() const
{
	return toString( 0 );
}

const EmaString& NoDataImpl::toString( UInt64 indent ) const
{
	addIndent( _toString.clear(), indent ).append( "NoData\n" );

	addIndent( _toString, indent ).append( "NoDataEnd\n" );

	return _toString;
}

const EmaBuffer& NoDataImpl::getAsHex() const
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}

Data::DataCode NoDataImpl::getCode() const
{
	return Data::NoCodeEnum;
}

Decoder& NoDataImpl::getDecoder()
{
	return *this;
}

bool NoDataImpl::hasDecoder() const
{
	return true;
}

bool NoDataImpl::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool NoDataImpl::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* , void* )
{
	_majVer = majVer;

	_minVer = minVer;
	
	_rsslBuffer = *rsslBuffer;

	return true;
}

bool NoDataImpl::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

const Encoder& NoDataImpl::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

const RsslBuffer& NoDataImpl::getRsslBuffer() const
{
	return _rsslBuffer;
}

bool NoDataImpl::hasEncoder() const
{
	return false;
}

OmmError::ErrorCode NoDataImpl::getErrorCode() const
{
	return _errorCode;
}
