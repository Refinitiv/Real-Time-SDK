/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

#include "OmmJsonEncoder.h"

using namespace refinitiv::ema::access;

OmmJsonEncoder::OmmJsonEncoder()
{
	_containerComplete = true;
}

OmmJsonEncoder::~OmmJsonEncoder()
{
}

void OmmJsonEncoder::set( const EmaBuffer& value )
{
	encodeBuffer( value.c_buf(), value.length() );
}

void OmmJsonEncoder::set( const EmaString& value )
{
	encodeBuffer( value.c_str(), value.length() );
}
