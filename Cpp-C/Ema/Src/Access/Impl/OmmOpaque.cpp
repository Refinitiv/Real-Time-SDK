/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmOpaque.h"
#include "OmmOpaqueDecoder.h"
#include "OmmOpaqueEncoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include "GlobalPool.h"
#include <new>

using namespace refinitiv::ema::access;

OmmOpaque::OmmOpaque() :
 _toString(),
 _pDecoder( new ( _space ) OmmOpaqueDecoder() ),
 _pEncoder ( 0 )
{
}

OmmOpaque& OmmOpaque::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

OmmOpaque::~OmmOpaque()
{
	_pDecoder->~OmmOpaqueDecoder();

	if ( _pEncoder )
	{
		g_pool.returnItem( _pEncoder );
	}
}

DataType::DataTypeEnum OmmOpaque::getDataType() const
{
	return DataType::OpaqueEnum;
}

Data::DataCode OmmOpaque::getCode() const
{
	return _pDecoder->getCode();
}

const EmaString& OmmOpaque::toString() const
{
	return toString( 0 );
}

const EmaString& OmmOpaque::toString( UInt64 indent ) const
{
	addIndent( _toString.clear(), indent ).append( "Opaque\n\n" ).append( getAsHex() );
	addIndent( _toString.append( "\n" ), indent ).append( "OpaqueEnd\n" );
	return _toString;
}

const EmaBuffer& OmmOpaque::getAsHex() const
{
	return _pDecoder->getBuffer();
}

const EmaBuffer& OmmOpaque::getBuffer() const
{
	return _pDecoder->getBuffer();
}

Decoder& OmmOpaque::getDecoder()
{
	return *_pDecoder;
}

bool OmmOpaque::hasDecoder() const
{
	return true;
}

OmmOpaque& OmmOpaque::set( const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getOmmOpaqueEncoderItem();

	_pEncoder->set( value );

	return *this;
}

OmmOpaque& OmmOpaque::set( const EmaString& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getOmmOpaqueEncoderItem();

	_pEncoder->set( value );

	return *this;
}

const EmaString& OmmOpaque::getString() const
{
	return _pDecoder->getString();
}

const Encoder& OmmOpaque::getEncoder() const
{
	if (!_pEncoder)
		_pEncoder = g_pool.getOmmOpaqueEncoderItem();

	return *_pEncoder;
}

bool OmmOpaque::hasEncoder() const
{
	return _pEncoder ? true : false;
}
