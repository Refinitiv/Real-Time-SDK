/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_EmaStringInt_h
#define __refinitiv_ema_access_EmaStringInt_h

#include "EmaString.h"

namespace rtsdk {

namespace ema {

namespace access {

class EmaStringInt : public EmaString
{
public :

	EmaStringInt();

	virtual ~EmaStringInt();

	void setInt( const char* , UInt32 , bool );

	void clear();

	const char* c_str() const;

	const EmaString& toString() const;

private :

	mutable char*	_pTempString;
	mutable bool	_nullTerminated;
};

}

}

}

#endif //__refinitiv_ema_access_EmaStringInt_h
