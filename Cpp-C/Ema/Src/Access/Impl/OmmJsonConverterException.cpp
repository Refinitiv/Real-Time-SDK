/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "OmmJsonConverterException.h"
#include "EmaStringInt.h"
#include "Utilities.h"

#include <stdio.h>
#include <string.h>

using namespace refinitiv::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

OmmJsonConverterException::OmmJsonConverterException() :
	OmmException()
{
	_errorCode = NoErrorEnum;
}

OmmJsonConverterException::~OmmJsonConverterException()
{
}

OmmJsonConverterException::OmmJsonConverterException(const OmmJsonConverterException& other) :
	OmmException(other)
{
	_errorCode = other._errorCode;
}

OmmJsonConverterException& OmmJsonConverterException::operator=(const OmmJsonConverterException& other)
{
	if (this == &other) return *this;

	OmmException::operator=(other);
	_errorCode = other._errorCode;

	return *this;
}

const EmaString& OmmJsonConverterException::toString() const
{
	int length = snprintf(_space + EMASTRING_SIZE, MAX_SIZE_PLUS_PADDING - EMASTRING_SIZE, "Exception Type='%s', Text='%s', ErrorCode='%d'",
		getExceptionTypeAsString().c_str(),
		_errorText + EMASTRING_SIZE,
		_errorCode); // Overrides this function to print error code as well.

	reinterpret_cast<EmaStringInt*>(_space)->setInt(_space + EMASTRING_SIZE, length, true);

	return *reinterpret_cast<const EmaString*>(_space);
}

Int32 OmmJsonConverterException::getErrorCode() const
{
	return _errorCode;
}

OmmException::ExceptionType OmmJsonConverterException::getExceptionType() const
{
	return OmmException::OmmJsonConverterExceptionEnum;
}

const EmaString& OmmJsonConverterException::getText() const
{
	reinterpret_cast<EmaStringInt*>(_errorText)->setInt(_errorText + EMASTRING_SIZE, _errorTextLength, true);
	return *reinterpret_cast<const EmaString*>(_errorText);
}
