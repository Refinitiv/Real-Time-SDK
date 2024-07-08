/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "EmaBufferInt.h"
#include "EmaString.h"
#include "Utilities.h"
#include <stdlib.h>

using namespace refinitiv::ema::access;

EmaBufferInt::EmaBufferInt() :
 EmaBuffer()
{
}

EmaBufferInt::~EmaBufferInt()
{
	_pBuffer = 0;
}

void EmaBufferInt::setFromInt( const char* buf, UInt32 length )
{
	_pBuffer = (char*)buf;
	_length = length;

	markDirty();
}

const EmaBuffer& EmaBufferInt::toBuffer() const
{
	return *this;
}

void EmaBufferInt::clear()
{
	_pBuffer = 0;
	_length = 0;

	markDirty();
}
