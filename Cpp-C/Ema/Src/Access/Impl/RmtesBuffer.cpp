/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "RmtesBuffer.h"
#include "RmtesBufferImpl.h"

#include <new>

using namespace thomsonreuters::ema::access;

RmtesBuffer::RmtesBuffer() :
 _pImpl( new ( _space ) RmtesBufferImpl() )
{
}

RmtesBuffer::RmtesBuffer( UInt32 length ) :
 _pImpl( new ( _space ) RmtesBufferImpl( length ) )
{
}

RmtesBuffer::RmtesBuffer( const char* buf, UInt32 length ) :
 _pImpl( new ( _space ) RmtesBufferImpl( buf, length ) )
{
}

RmtesBuffer::RmtesBuffer( const RmtesBuffer& rmtesBuffer ) :
 _pImpl( new ( _space ) RmtesBufferImpl( *rmtesBuffer._pImpl ) )
{
}

RmtesBuffer::~RmtesBuffer()
{
	_pImpl->~RmtesBufferImpl();
}

RmtesBuffer& RmtesBuffer::clear()
{
	_pImpl->clear();
	return *this;
}

const EmaBuffer& RmtesBuffer::getAsUTF8() const
{
	return _pImpl->getAsUTF8();
}

const EmaBufferU16& RmtesBuffer::getAsUTF16() const
{
	return _pImpl->getAsUTF16();
}

const EmaString& RmtesBuffer::toString() const
{
	return _pImpl->toString();
}

RmtesBuffer& RmtesBuffer::apply( const RmtesBuffer& source )
{
	_pImpl->apply( *source._pImpl );
	return *this;
}

RmtesBuffer& RmtesBuffer::apply( const char* source, UInt32 length )
{
	_pImpl->apply( source, length );
	return *this;
}

RmtesBuffer& RmtesBuffer::apply( const EmaBuffer& source )
{
	_pImpl->apply( source );
	return *this;
}
