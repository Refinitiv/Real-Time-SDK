/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#ifndef __thomsonreuters_ema_access_GetTime_h
#define __thomsonreuters_ema_access_GetTime_h

#include "Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class GetTime
{
public :

	static UInt64 getSeconds();
	static UInt64 getMillis();
	static UInt64 getMicros();
	static UInt64 getNanos();
	static UInt64 getTicks();

	static double ticksPerSecond();
	static double ticksPerMilli();
	static double ticksPerMicro();
	static double ticksPerNano();

private:

	static double			_initTicksPerSecond();
	static double			_initTicksPerMilli();
	static double			_initTicksPerMicro();
	static double			_initTicksPerNano();

	static const double		_TICKS_PER_SECOND;
	static const double		_TICKS_PER_MILLI;
	static const double		_TICKS_PER_MICRO;
	static const double		_TICKS_PER_NANO;
};

}

}

}

#endif // __thomsonreuters_ema_access_GetTime_h

