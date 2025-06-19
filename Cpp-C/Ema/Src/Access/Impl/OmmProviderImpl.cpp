/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmProviderImpl.h"

using namespace refinitiv::ema::access;

OmmProviderImpl::OmmProviderImpl(OmmProvider* ommProvider) :
	_pOmmProvider(ommProvider)
{
}

OmmProviderImpl::~OmmProviderImpl()
{
}
