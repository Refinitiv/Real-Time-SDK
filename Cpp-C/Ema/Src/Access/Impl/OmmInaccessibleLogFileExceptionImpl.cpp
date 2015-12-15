/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmInaccessibleLogFileExceptionImpl.h"

using namespace thomsonreuters::ema::access;

OmmInaccessibleLogFileExceptionImpl::OmmInaccessibleLogFileExceptionImpl( const EmaString& fileName)
	: OmmInaccessibleLogFileException( fileName )
{
}

OmmInaccessibleLogFileExceptionImpl::~OmmInaccessibleLogFileExceptionImpl()
{
}

OmmInaccessibleLogFileExceptionImpl::OmmInaccessibleLogFileExceptionImpl( const OmmInaccessibleLogFileExceptionImpl& other ) :
 OmmInaccessibleLogFileException( other )
{
}

OmmInaccessibleLogFileExceptionImpl& OmmInaccessibleLogFileExceptionImpl::operator=( const OmmInaccessibleLogFileExceptionImpl& other )
{
	if ( this == &other ) return *this;

	OmmInaccessibleLogFileException::operator=( other );
	
	return *this;
}

void OmmInaccessibleLogFileExceptionImpl::throwException( const EmaString& fileName, const EmaString& text )
{
	OmmInaccessibleLogFileExceptionImpl exception( fileName );

	exception.statusText( text );

	throw exception;
}

void OmmInaccessibleLogFileExceptionImpl::throwException( const EmaString& fileName,const char* text )
{
	OmmInaccessibleLogFileExceptionImpl exception( fileName );

	exception.statusText( text );

	throw exception;
}
