/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_FieldListSetDef_h
#define __thomsonreuters_ema_access_FieldListSetDef_h

#include "EmaPool.h"

#include "rtr/rsslSetData.h"

namespace thomsonreuters {

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

class FieldListSetDefPool : public Pool< FieldListSetDef >
{
public :

	FieldListSetDefPool( unsigned int size = 5 ) : Pool< FieldListSetDef >( size ) {};

	virtual ~FieldListSetDefPool() {}

private :

	FieldListSetDefPool( const FieldListSetDefPool& );
	FieldListSetDefPool& operator=( const FieldListSetDefPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_FieldListSetDef_h
