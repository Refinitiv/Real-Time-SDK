/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmAnsiPageEncoder.h"

using namespace refinitiv::ema::access;

OmmAnsiPageEncoder::OmmAnsiPageEncoder(void)
{
	_containerComplete = true;
}

OmmAnsiPageEncoder::~OmmAnsiPageEncoder(void)
{
}

void OmmAnsiPageEncoder::set( const EmaBuffer& value )
{
	encodeBuffer( value.c_buf(), value.length() );
}

void OmmAnsiPageEncoder::set( const EmaString& value )
{
	encodeBuffer( value.c_str(), value.length() );
}
