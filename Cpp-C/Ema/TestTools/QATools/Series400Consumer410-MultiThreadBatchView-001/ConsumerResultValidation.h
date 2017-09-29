///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __ema_resultvalidatio_h_
#define __ema_resultvalidatio_h_

#include <iostream>
#include <list>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "Ema.h"

using namespace std;

class ResultValidation
{
public:
	ResultValidation(const thomsonreuters::ema::access::EmaString& itemName);

	bool closureValidate(const thomsonreuters::ema::access::EmaString& receivedItemName);

	static void printTestResult();

	static int _numRequestOpen;
	static int _numRefreshReceived;
	static int _numUpdateReceived;
	static int _numStatusReceived;
	static int _numInvalidClosure;
	static int _numValidClosure;


	static bool _SNAPSHOT;
	static int _NUMOFITEMPERLOOP;
	static bool _USERDISPATCH;
	static int _USERDISPATCHTIMEOUT;
	static int _RUNTIME;
	static std::list<void*> closuresList;

protected:

	thomsonreuters::ema::access::EmaString _itemName;
};

#endif // __ema_resultvalidatio_h_
