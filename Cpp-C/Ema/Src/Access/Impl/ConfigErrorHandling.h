/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_ConfigErrorHandling_h
#define __rtsdk_ema_access_ConfigErrorHandling_h

#include "EmaString.h"
#include "OmmLoggerClient.h"

namespace rtsdk {

namespace ema {

namespace access {

class EmaConfigError
{
public:

	EmaConfigError( const EmaString& errorMsg, OmmLoggerClient::Severity severity );

	virtual ~EmaConfigError();

	OmmLoggerClient::Severity severity() const;

	const EmaString& errorMsg() const;

private:

	OmmLoggerClient::Severity	_severity;
	EmaString					_errorMsg;
};

class EmaConfigErrorList
{
public:

	EmaConfigErrorList();

	virtual ~EmaConfigErrorList();

	void add( EmaConfigError* );

	void add( EmaConfigErrorList& );

	void printErrors( OmmLoggerClient::Severity severity = OmmLoggerClient::VerboseEnum );

	void log( OmmLoggerClient*, OmmLoggerClient::Severity = OmmLoggerClient::VerboseEnum );

	void clear();

	UInt32 count() const;

private:

	struct ListElement
	{
		ListElement( EmaConfigError* error ) : error( error ), next( 0 ) {}

		EmaConfigError*		error;
		ListElement*		next;
	};

	ListElement*	_pList;
	UInt32			_count;
};

}

}

}

#endif //__rtsdk_ema_access_ConfigErrorHandling_h
