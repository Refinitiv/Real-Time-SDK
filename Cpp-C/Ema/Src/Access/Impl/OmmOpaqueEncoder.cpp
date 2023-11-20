/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmOpaqueEncoder.h"
#include "EmaBuffer.h"

using namespace refinitiv::ema::access;

OmmOpaqueEncoder::OmmOpaqueEncoder()
{
	_containerComplete = true;
}

OmmOpaqueEncoder::~OmmOpaqueEncoder()
{
}

void OmmOpaqueEncoder::set( const EmaBuffer& value )
{
	encodeBuffer( value.c_buf(), value.length() );
}

void OmmOpaqueEncoder::set( const EmaString& value )
{
	encodeBuffer( value.c_str(), value.length() );
}
