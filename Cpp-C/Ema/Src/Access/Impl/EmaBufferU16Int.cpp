/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "EmaBufferU16Int.h"

using namespace refinitiv::ema::access;

EmaBufferU16Int::EmaBufferU16Int() :
 EmaBufferU16()
{
}

EmaBufferU16Int::~EmaBufferU16Int()
{
	_pBuffer = 0;
}

void EmaBufferU16Int::setFromInt( const UInt16* buf, UInt32 length )
{
	_pBuffer = (UInt16*)buf;
	_length = length;
}

const EmaBufferU16& EmaBufferU16Int::toBuffer() const
{
	return *this;
}

void EmaBufferU16Int::clear()
{
	_pBuffer = 0;
	_length = 0;
}
