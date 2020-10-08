/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ElementListSetDef_h
#define __refinitiv_ema_access_ElementListSetDef_h

#include "EmaPool.h"

#include "rtr/rsslSetData.h"

namespace refinitiv {

namespace ema {

namespace access {

class ElementListSetDef
{
public :

	ElementListSetDef();

	virtual ~ElementListSetDef();

	RsslLocalElementSetDefDb* getSetDefDb();

private :

	RsslLocalElementSetDefDb		_rsslElementListSetDb;

	ElementListSetDef( const ElementListSetDef& );
	ElementListSetDef& operator=( const ElementListSetDef& );
};

class ElementListSetDefPool : public Pool< ElementListSetDef >
{
public :

	ElementListSetDefPool( unsigned int size = 5 ) : Pool< ElementListSetDef >( size ) {};

	virtual ~ElementListSetDefPool() {}

private :

	ElementListSetDefPool( const ElementListSetDefPool& );
	ElementListSetDefPool& operator=( const ElementListSetDefPool& );
};

}

}

}

#endif // __refinitiv_ema_access_ElementListSetDef_h
