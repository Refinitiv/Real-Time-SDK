/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_StreamId_h
#define __thomsonreuters_ema_access_StreamId_h

#include "EmaList.h"

namespace thomsonreuters {
	
namespace ema {

namespace access {

class StreamId : public ListLinks< StreamId >
{
public:

    StreamId( Int32 );
    Int32 operator()() const;

private:

    Int32 value;
};

}

}

}

#endif // __thomsonreuters_ema_access_StreamId_h

