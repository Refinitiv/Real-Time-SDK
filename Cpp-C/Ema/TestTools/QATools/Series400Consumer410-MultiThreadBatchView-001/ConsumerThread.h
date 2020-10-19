///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __ema_consumerthread_h_
#define __ema_consumerthread_h_

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

class AppThread
{
public:
	AppThread(refinitiv::ema::access::OmmConsumer*);

	virtual ~AppThread();

	void start();

	void stop();

	virtual void run();

protected:
	refinitiv::ema::access::OmmConsumer* _ommConsumer;
	bool _running;

#ifdef WIN32
	static unsigned __stdcall ThreadFunc(void* pArguments);

	HANDLE					_handle;
	unsigned int			_threadId;
#else
	static void *ThreadFunc(void* pArguments);

	pthread_t				_threadId;
#endif

};

class ConsumerThread : public AppThread
{
public :

	ConsumerThread(refinitiv::ema::access::OmmConsumer*, 
					refinitiv::ema::access::OmmConsumerClient* client);

	virtual ~ConsumerThread();

	void run();

	void openItem(const refinitiv::ema::access::EmaString& item, 
				const refinitiv::ema::access::EmaString& serviceName,
	            unsigned int mode);

	void sendRequest();

protected :
	refinitiv::ema::access::EmaString _itemNamePrefix;
	refinitiv::ema::access::EmaString _serviceName;
	unsigned int   _mode;
	refinitiv::ema::access::ReqMsg _reqMsg;
	refinitiv::ema::access::OmmConsumerClient* _client;
};

#endif // __ema_consumerthread_h_
