/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ConfigErrorHandling_h
#define __thomsonreuters_ema_access_ConfigErrorHandling_h

#include "EmaString.h"
#include "OmmLoggerClient.h"

class EmaConfigError
{
public:

	EmaConfigError( const thomsonreuters::ema::access::EmaString & errorMsg, thomsonreuters::ema::access::OmmLoggerClient::Severity severity ) : _errorMsg( errorMsg ), _severity( severity ) {}

	thomsonreuters::ema::access::OmmLoggerClient::Severity severity()
	{
		return _severity;
	}

	const thomsonreuters::ema::access::EmaString& errorMsg()
	{
		return _errorMsg;
	}
	
private:
	thomsonreuters::ema::access::OmmLoggerClient::Severity	_severity;
	thomsonreuters::ema::access::EmaString					_errorMsg;
};

class EmaConfigErrorList {
public:
	EmaConfigErrorList() : theList(0), _count(0) {}
	~EmaConfigErrorList()
	{
		clear();
	}
	void add(EmaConfigError *);
	void add( EmaConfigErrorList & );
	void printErrors(thomsonreuters::ema::access::OmmLoggerClient::Severity severity = thomsonreuters::ema::access::OmmLoggerClient::VerboseEnum);
	void log(thomsonreuters::ema::access::OmmLoggerClient *, thomsonreuters::ema::access::OmmLoggerClient::Severity = thomsonreuters::ema::access::OmmLoggerClient::VerboseEnum);
	void clear();
	int count() { return _count; }
private:
	struct listElement {
		listElement(EmaConfigError * error) : error(error), next(0) {}
		EmaConfigError * error;
		listElement * next;
	};
	listElement * theList;
	int _count;
};

#endif //__thomsonreuters_ema_access_DefaultXML_h
