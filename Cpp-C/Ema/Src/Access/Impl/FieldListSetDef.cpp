/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "FieldListSetDef.h"
#include <new>

#define FL_SET_DEF_SIZE 12000

using namespace refinitiv::ema::access;

FieldListSetDef::FieldListSetDef()
{
	rsslClearLocalFieldSetDefDb( &_rsslFieldListSetDb );

	_rsslFieldListSetDb.entries.data = new char[ FL_SET_DEF_SIZE ];
	_rsslFieldListSetDb.entries.length = FL_SET_DEF_SIZE;
}

FieldListSetDef::~FieldListSetDef()
{
	delete [] _rsslFieldListSetDb.entries.data;
}

RsslLocalFieldSetDefDb* FieldListSetDef::getSetDefDb()
{
	return &_rsslFieldListSetDb;
}
