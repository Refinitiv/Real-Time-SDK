/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmConsumerActiveConfig_h
#define __refinitiv_ema_access_OmmConsumerActiveConfig_h

#include "OmmConsumerConfig.h"
#include "ActiveConfig.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmConsumerActiveConfig : public ActiveConfig
{
public :

	OmmConsumerActiveConfig();

	virtual ~OmmConsumerActiveConfig();

	void clear();

	OmmConsumerConfig::OperationModel		operationModel;
};

}

}

}

#endif // __refinitiv_ema_access_OmmConsumerActiveConfig_h
