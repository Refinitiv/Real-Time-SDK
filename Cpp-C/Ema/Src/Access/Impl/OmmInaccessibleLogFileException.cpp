/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "OmmInaccessibleLogFileException.h"
#include "EmaStringInt.h"

using namespace refinitiv::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

OmmInaccessibleLogFileException::OmmInaccessibleLogFileException( const EmaString& fileName ) :
 OmmException(),
 _fileName( fileName )
{
}


OmmInaccessibleLogFileException::~OmmInaccessibleLogFileException()
{
}

OmmInaccessibleLogFileException::OmmInaccessibleLogFileException( const OmmInaccessibleLogFileException& other ) :
 OmmException( other ),
 _fileName( other._fileName )
{
}

OmmInaccessibleLogFileException& OmmInaccessibleLogFileException::operator=( const OmmInaccessibleLogFileException& other )
{
	if ( this == &other ) return *this;

	OmmException::operator=( other );

	_fileName = other._fileName;

	return *this;
}

const EmaString& OmmInaccessibleLogFileException::toString() const
{
	return toStringInt();
}

OmmException::ExceptionType OmmInaccessibleLogFileException::getExceptionType() const
{
	return OmmException::OmmInaccessibleLogFileExceptionEnum;
}

const EmaString& OmmInaccessibleLogFileException::getText() const
{
	reinterpret_cast<EmaStringInt*>(_errorText)->setInt( _errorText + EMASTRING_SIZE, _errorTextLength, true );
	return *reinterpret_cast<const EmaString*>( _errorText );
}

const EmaString& OmmInaccessibleLogFileException::getFilename() const
{
	return _fileName;
}
