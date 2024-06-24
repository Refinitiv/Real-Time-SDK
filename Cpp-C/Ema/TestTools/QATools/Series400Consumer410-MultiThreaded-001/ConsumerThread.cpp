///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "ConsumerThread.h"
#include "ConsumerMutex.h"
#include "ConsumerResultValidation.h"

using namespace refinitiv::ema::access;

void sleep(int millisecs)
{
#if defined WIN32
	::Sleep((DWORD)(millisecs));
#else
	struct timespec sleeptime;
	sleeptime.tv_sec = millisecs / 1000;
	sleeptime.tv_nsec = (millisecs % 1000) * 1000000;
	nanosleep(&sleeptime, 0);
#endif
}
static Mutex _userLock;

AppThread::AppThread(refinitiv::ema::access::OmmConsumer* ommConsumer) :
	_running(false),
	_ommConsumer(ommConsumer)
{
}

AppThread::~AppThread()
{
	stop();
}

void AppThread::run()
{
	while (_running)
	{
		_ommConsumer->dispatch(ResultValidation::_USERDISPATCHTIMEOUT);
	}
}

#ifdef WIN32
unsigned __stdcall AppThread::ThreadFunc(void* pArguments)
{
	((AppThread *)pArguments)->run();

	return 0;
}

#else
extern "C"
{
	void * AppThread::ThreadFunc(void* pArguments)
	{
		((AppThread *)pArguments)->run();
	}
}
#endif

void  AppThread::start()
{
#ifdef WIN32
	_handle = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, this, 0, &_threadId);
	assert(_handle != 0);

	SetThreadPriority(_handle, THREAD_PRIORITY_NORMAL);
#else
	pthread_create(&_threadId, NULL, ThreadFunc, this);
	assert(_threadId != 0);
#endif

	_running = true;
}

void  AppThread::stop()
{
	if (_running)
	{
		_running = false;
#ifdef WIN32
		WaitForSingleObject(_handle, INFINITE);
		CloseHandle(_handle);
		_handle = 0;
		_threadId = 0;
#else
		pthread_join(_threadId, NULL);
		_threadId = 0;
#endif
	}

	_running = false;
}

ConsumerThread::ConsumerThread(refinitiv::ema::access::OmmConsumer* ommConsumer,
	refinitiv::ema::access::OmmConsumerClient* client) :
AppThread(ommConsumer),
 _reqMsg(),
_client(client)
{
}

ConsumerThread::~ConsumerThread()
{
}

void ConsumerThread::openItem(const refinitiv::ema::access::EmaString& item, 
							 const refinitiv::ema::access::EmaString& serviceName)
{
	_itemNamePrefix = item;
	_serviceName = serviceName;
}

void ConsumerThread::sendRequest()
{
	EmaString itemStrBuilder;
	while (_running)
	{
		sleep(1000);

		for (int idx = 1; idx <= ResultValidation::_NUMOFITEMPERLOOP; ++idx)
		{
			itemStrBuilder.clear();

			EmaString itemName = itemStrBuilder.append(_itemNamePrefix).append(idx);
		
			_reqMsg.clear().name(itemName).serviceName(_serviceName).interestAfterRefresh(!ResultValidation::_SNAPSHOT);
			
			_ommConsumer->registerClient(_reqMsg, *_client, (void *)(new ResultValidation(itemName)));

			_userLock.lock();
			++ResultValidation::_numRequestOpen;
			_userLock.unlock();
		}
	}
}

void ConsumerThread::run()
{
	sendRequest();
}
