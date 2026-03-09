/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmInvalidUsageExceptionImpl_h
#define __refinitiv_ema_access_OmmInvalidUsageExceptionImpl_h

#include "OmmInvalidUsageException.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmInvalidUsageExceptionImpl : public OmmInvalidUsageException
{
public :

	static void throwException( const EmaString&, Int32 );

	static void throwException( const char*, Int32 );

	static OmmInvalidUsageExceptionImpl makeException( const char*, Int32 );

	OmmInvalidUsageExceptionImpl();

	virtual ~OmmInvalidUsageExceptionImpl();

private :

	OmmInvalidUsageExceptionImpl( const OmmInvalidUsageExceptionImpl& );
	OmmInvalidUsageExceptionImpl& operator=( const OmmInvalidUsageExceptionImpl& );

	friend class NoDataImpl;

	friend class OmmAnsiPage;
	friend class OmmAscii;
	friend class OmmBuffer;
	friend class OmmDate;
	friend class OmmDateTime;
	friend class OmmDouble;
	friend class OmmEnum;
	friend class OmmError;
	friend class OmmFloat;
	friend class OmmInt;
	friend class OmmJson;
	friend class OmmQos;
	friend class OmmReal;
	friend class OmmRmtes;
	friend class OmmState;
	friend class OmmTime;
	friend class OmmUInt;
	friend class OmmUtf8;
	friend class OmmXml;
};

}

}

}

#endif // __refinitiv_ema_access_OmmInvalidUsageExceptionImpl_h
