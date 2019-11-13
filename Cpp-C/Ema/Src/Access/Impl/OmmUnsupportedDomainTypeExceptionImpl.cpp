/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmUnsupportedDomainTypeExceptionImpl.h"

using namespace thomsonreuters::ema::access;

OmmUnsupportedDomainTypeExceptionImpl::OmmUnsupportedDomainTypeExceptionImpl()
{
}

OmmUnsupportedDomainTypeExceptionImpl::~OmmUnsupportedDomainTypeExceptionImpl()
{
}

OmmUnsupportedDomainTypeExceptionImpl::OmmUnsupportedDomainTypeExceptionImpl( const OmmUnsupportedDomainTypeExceptionImpl& other ) :
 OmmUnsupportedDomainTypeException( other )
{
}

OmmUnsupportedDomainTypeExceptionImpl& OmmUnsupportedDomainTypeExceptionImpl::operator=( const OmmUnsupportedDomainTypeExceptionImpl& other )
{
	if ( this == &other ) return *this;

	OmmUnsupportedDomainTypeException::operator=( other );

	return *this;
}

void OmmUnsupportedDomainTypeExceptionImpl::throwException( const EmaString& text, UInt16 domainType )
{
	OmmUnsupportedDomainTypeExceptionImpl exception;

	exception.statusText( text );
	exception._domainType = domainType;

	throw exception;
}
