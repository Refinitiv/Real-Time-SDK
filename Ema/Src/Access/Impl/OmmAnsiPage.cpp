/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmAnsiPage.h"
#include "OmmAnsiPageDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include "GlobalPool.h"
#include <new>

using namespace thomsonreuters::ema::access;

OmmAnsiPage::OmmAnsiPage() :
 _toString(),
 _pDecoder( new ( _space ) OmmAnsiPageDecoder() ),
 _pEncoder ( 0 )
{
}

OmmAnsiPage::~OmmAnsiPage()
{
	_pDecoder->~OmmAnsiPageDecoder();

	if ( _pEncoder )
	{
		g_pool._ommAnsiPageEncoderPool.returnItem( _pEncoder );
		_pEncoder = 0;
	}
}

DataType::DataTypeEnum OmmAnsiPage::getDataType() const
{
	return DataType::AnsiPageEnum;
}

Data::DataCode OmmAnsiPage::getCode() const
{
	return _pDecoder->getCode();
}

const EmaBuffer& OmmAnsiPage::getAsHex() const
{
	return _pDecoder->getBuffer();
}

const EmaString& OmmAnsiPage::getString() const
{
	return _pDecoder->getString();
}

const EmaBuffer& OmmAnsiPage::getBuffer() const
{
	return _pDecoder->getBuffer();
}

OmmAnsiPage& OmmAnsiPage::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

OmmAnsiPage& OmmAnsiPage::set( const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ommAnsiPageEncoderPool.getItem();

	_pEncoder->set( value );

	return *this;
}

OmmAnsiPage& OmmAnsiPage::set( const EmaString& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ommAnsiPageEncoderPool.getItem();

	_pEncoder->set( value );

	return *this;
}

const EmaString& OmmAnsiPage::toString() const
{
	return toString( 0 );
}

const EmaString& OmmAnsiPage::toString( UInt64 indent ) const
{
	addIndent( _toString.clear(), indent ).append( "AnsiPage\n\n" ).append( getAsHex() ).append( "\n" );
	addIndent( _toString, indent ).append( "AnsiPageEnd" );
	return _toString;
}

Decoder& OmmAnsiPage::getDecoder()
{
	return *_pDecoder;
}

const Encoder& OmmAnsiPage::getEncoder() const
{
	return *static_cast<const Encoder*>( _pEncoder );
}
