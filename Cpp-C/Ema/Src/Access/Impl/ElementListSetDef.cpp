/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ElementListSetDef.h"
#include <new>

#define EL_SET_DEF_SIZE 50000

using namespace refinitiv::ema::access;

ElementListSetDef::ElementListSetDef()
{
	rsslClearLocalElementSetDefDb( &_rsslElementListSetDb );

	_rsslElementListSetDb.entries.data = new char[ EL_SET_DEF_SIZE ];
	_rsslElementListSetDb.entries.length = EL_SET_DEF_SIZE;
}

ElementListSetDef::~ElementListSetDef()
{
	delete [] _rsslElementListSetDb.entries.data;
}

RsslLocalElementSetDefDb* ElementListSetDef::getSetDefDb()
{
	return &_rsslElementListSetDb;
}
