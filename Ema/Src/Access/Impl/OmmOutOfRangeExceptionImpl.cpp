/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmOutOfRangeExceptionImpl.h"

using namespace thomsonreuters::ema::access;

OmmOutOfRangeExceptionImpl::OmmOutOfRangeExceptionImpl()
{
}

OmmOutOfRangeExceptionImpl::~OmmOutOfRangeExceptionImpl()
{
}

OmmOutOfRangeExceptionImpl::OmmOutOfRangeExceptionImpl( const OmmOutOfRangeExceptionImpl& other ) :
 OmmOutOfRangeException( other )
{
}

OmmOutOfRangeExceptionImpl& OmmOutOfRangeExceptionImpl::operator=( const OmmOutOfRangeExceptionImpl& other )
{
	if ( this == &other ) return *this;

	OmmOutOfRangeException::operator=( other );

	return *this;
}

void OmmOutOfRangeExceptionImpl::throwException( const EmaString& text )
{
	OmmOutOfRangeExceptionImpl exception;

	exception.statusText( text );

	throw exception;
}
