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
	AppThread(rtsdk::ema::access::OmmConsumer*);

	virtual ~AppThread();

	void start();

	void stop();

	virtual void run();

protected:
	rtsdk::ema::access::OmmConsumer* _ommConsumer;
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

	ConsumerThread(rtsdk::ema::access::OmmConsumer*, 
					rtsdk::ema::access::OmmConsumerClient* client);

	virtual ~ConsumerThread();

	void run();

	void openItem(const rtsdk::ema::access::EmaString& item, 
				const rtsdk::ema::access::EmaString& serviceName);

	void sendRequest();

protected :
	rtsdk::ema::access::EmaString _itemNamePrefix;
	rtsdk::ema::access::EmaString _serviceName;
	rtsdk::ema::access::ReqMsg _reqMsg;
	rtsdk::ema::access::OmmConsumerClient* _client;
};

#endif // __ema_consumerthread_h_
