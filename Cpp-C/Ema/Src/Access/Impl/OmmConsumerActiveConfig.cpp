/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumerActiveConfig.h"

using namespace refinitiv::ema::access;

#define DEFAULT_USER_DISPATCH OmmConsumerConfig::ApiDispatchEnum
static const EmaString DEFAULT_CONSUMER_SERVICE_NAME( "14002" );

OmmConsumerActiveConfig::OmmConsumerActiveConfig() :
	ActiveConfig( DEFAULT_CONSUMER_SERVICE_NAME ),
	operationModel( DEFAULT_USER_DISPATCH )
{
}

OmmConsumerActiveConfig::~OmmConsumerActiveConfig()
{
}

void OmmConsumerActiveConfig::clear()
{
	ActiveConfig::clear();
}
