/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmMemoryExhaustionExceptionImpl.h"

using namespace refinitiv::ema::access;

OmmMemoryExhaustionExceptionImpl::OmmMemoryExhaustionExceptionImpl()
{
}

OmmMemoryExhaustionExceptionImpl::~OmmMemoryExhaustionExceptionImpl()
{
}

OmmMemoryExhaustionExceptionImpl::OmmMemoryExhaustionExceptionImpl( const OmmMemoryExhaustionExceptionImpl& other ) :
 OmmMemoryExhaustionException( other )
{
}

OmmMemoryExhaustionExceptionImpl& OmmMemoryExhaustionExceptionImpl::operator=( const OmmMemoryExhaustionExceptionImpl& other )
{
	if ( this == &other ) return *this;

	OmmMemoryExhaustionException::operator=( other );
	
	return *this;
}

void OmmMemoryExhaustionExceptionImpl::throwException( const char* text )
{
	OmmMemoryExhaustionExceptionImpl exception;

	exception.statusText( text );

	throw exception;
}
