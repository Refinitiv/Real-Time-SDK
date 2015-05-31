/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_EmaStringInt_h
#define __thomsonreuters_ema_access_EmaStringInt_h

#include "EmaString.h"

namespace thomsonreuters {

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

#endif //__thomsonreuters_ema_access_EmaStringInt_h
