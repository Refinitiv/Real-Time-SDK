/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmJson.h"
#include "OmmJsonDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include "GlobalPool.h"
#include <new>

using namespace refinitiv::ema::access;

OmmJson::OmmJson() :
 _toString(),
 _pDecoder( new ( _space ) OmmJsonDecoder() ),
 _pEncoder ( 0 )
{
}

OmmJson::~OmmJson()
{
	_pDecoder->~OmmJsonDecoder();

	if ( _pEncoder )
	{
		g_pool.returnItem( _pEncoder );
	}
}

OmmJson& OmmJson::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

DataType::DataTypeEnum OmmJson::getDataType() const
{
	return DataType::JsonEnum;
}

Data::DataCode OmmJson::getCode() const
{
	return _pDecoder->getCode();
}

const EmaString& OmmJson::toString() const
{
	return toString( 0 );
}

const EmaString& OmmJson::toString( UInt64 indent ) const
{
	addIndent( _toString.clear(), indent ).append( "Json" );

	++indent;
	addIndent( _toString.append( "\n" ), indent ).append( _pDecoder->toString() );
	--indent;

	addIndent( _toString.append( "\n" ), indent ).append( "JsonEnd\n" );

	return _toString;
}

const EmaBuffer& OmmJson::getAsHex() const
{
	return _pDecoder->getBuffer();
}

const EmaString& OmmJson::getString() const
{
	return _pDecoder->getString();
}

const EmaBuffer& OmmJson::getBuffer() const
{
	return _pDecoder->getBuffer();
}

OmmJson& OmmJson::set( const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getOmmJsonEncoderItem();

	_pEncoder->set( value );

	return *this;
}

OmmJson& OmmJson::set( const EmaString& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getOmmJsonEncoderItem();

	_pEncoder->set( value );

	return *this;
}

Decoder& OmmJson::getDecoder()
{
	return *_pDecoder;
}

bool OmmJson::hasDecoder() const
{
	return true;
}

const Encoder& OmmJson::getEncoder() const
{
	return *static_cast<const Encoder*>( _pEncoder );
}

bool OmmJson::hasEncoder() const
{
	return _pEncoder ? true : false;
}
