/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmXml.h"
#include "OmmXmlDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include "GlobalPool.h"
#include <new>

using namespace rtsdk::ema::access;

OmmXml::OmmXml() :
 _toString(),
 _pDecoder( new ( _space ) OmmXmlDecoder() ),
 _pEncoder ( 0 )
{
}

OmmXml::~OmmXml()
{
	_pDecoder->~OmmXmlDecoder();

	if ( _pEncoder )
	{
		g_pool._ommXmlEncoderPool.returnItem( _pEncoder );
	}
}

OmmXml& OmmXml::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

DataType::DataTypeEnum OmmXml::getDataType() const
{
	return DataType::XmlEnum;
}

Data::DataCode OmmXml::getCode() const
{
	return _pDecoder->getCode();
}

const EmaString& OmmXml::toString() const
{
	return toString( 0 );
}

const EmaString& OmmXml::toString( UInt64 indent ) const
{
	addIndent( _toString.clear(), indent ).append( "Xml" );

	++indent;
	addIndent( _toString.append( "\n" ), indent ).append( _pDecoder->toString() );
	--indent;

	addIndent( _toString.append( "\n" ), indent ).append( "XmlEnd\n" );

	return _toString;
}

const EmaBuffer& OmmXml::getAsHex() const
{
	return _pDecoder->getBuffer();
}

const EmaString& OmmXml::getString() const
{
	return _pDecoder->getString();
}

const EmaBuffer& OmmXml::getBuffer() const
{
	return _pDecoder->getBuffer();
}

OmmXml& OmmXml::set( const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ommXmlEncoderPool.getItem();

	_pEncoder->set( value );

	return *this;
}

OmmXml& OmmXml::set( const EmaString& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ommXmlEncoderPool.getItem();

	_pEncoder->set( value );

	return *this;
}

Decoder& OmmXml::getDecoder()
{
	return *_pDecoder;
}

bool OmmXml::hasDecoder() const
{
	return true;
}

const Encoder& OmmXml::getEncoder() const
{
	return *static_cast<const Encoder*>( _pEncoder );
}

bool OmmXml::hasEncoder() const
{
	return _pEncoder ? true : false;
}
