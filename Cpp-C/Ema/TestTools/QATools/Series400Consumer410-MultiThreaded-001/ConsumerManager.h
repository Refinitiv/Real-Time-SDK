///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __ema_consumermanager_h_
#define __ema_consumermanager_h_

#include <assert.h>

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <process.h>
#include <windows.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "Ema.h"

class ConsumerManager
{
public :

	ConsumerManager( const thomsonreuters::ema::access::EmaString& host, const thomsonreuters::ema::access::EmaString& username );

	virtual ~ConsumerManager();

	thomsonreuters::ema::access::OmmConsumer& getOmmConsumer();

	void start();

	void stop();

	void run();

protected :

	bool running;

	thomsonreuters::ema::access::OmmConsumer* ommConsumer;

#ifdef WIN32
	static unsigned __stdcall ThreadFunc( void* pArguments );

	HANDLE					_handle;
	unsigned int			_threadId;
#else
	static void *ThreadFunc( void* pArguments );

	pthread_t				_threadId;
#endif
};

#endif // __ema_consumermanager_h_
