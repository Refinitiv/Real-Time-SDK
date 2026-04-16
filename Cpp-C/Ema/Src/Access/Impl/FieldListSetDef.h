/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_FieldListSetDef_h
#define __refinitiv_ema_access_FieldListSetDef_h

#include "rtr/rsslSetData.h"

namespace refinitiv {

namespace ema {

namespace access {

class FieldListSetDef
{
public :

	FieldListSetDef();

	virtual ~FieldListSetDef();

	RsslLocalFieldSetDefDb* getSetDefDb();

private :

	RsslLocalFieldSetDefDb		_rsslFieldListSetDb;

	FieldListSetDef( const FieldListSetDef& );
	FieldListSetDef& operator=( const FieldListSetDef& );
};

}

}

}

#endif // __refinitiv_ema_access_FieldListSetDef_h
