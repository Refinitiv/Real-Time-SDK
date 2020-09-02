/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmInvalidConfigurationExceptionImpl.h"

using namespace rtsdk::ema::access;

OmmInvalidConfigurationExceptionImpl::OmmInvalidConfigurationExceptionImpl()
{
}

OmmInvalidConfigurationExceptionImpl::~OmmInvalidConfigurationExceptionImpl()
{
}

OmmInvalidConfigurationExceptionImpl::OmmInvalidConfigurationExceptionImpl( const OmmInvalidConfigurationExceptionImpl& other ) :
 OmmInvalidConfigurationException( other )
{
}

OmmInvalidConfigurationExceptionImpl& OmmInvalidConfigurationExceptionImpl::operator=( const OmmInvalidConfigurationExceptionImpl& other )
{
	if ( this == &other ) return *this;

	OmmInvalidConfigurationException::operator=( other );

	return *this;
}

void OmmInvalidConfigurationExceptionImpl::throwException( const EmaString& text )
{
	OmmInvalidConfigurationExceptionImpl exception;

	exception.statusText( text );

	throw exception;
}
