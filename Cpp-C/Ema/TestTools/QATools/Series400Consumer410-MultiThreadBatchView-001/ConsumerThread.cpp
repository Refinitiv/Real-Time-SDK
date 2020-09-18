///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "ConsumerThread.h"
#include "ConsumerMutex.h"
#include "ConsumerResultValidation.h"

using namespace rtsdk::ema::access;
using namespace rtsdk::ema::rdm;

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

AppThread::AppThread(rtsdk::ema::access::OmmConsumer* ommConsumer) :
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

ConsumerThread::ConsumerThread(rtsdk::ema::access::OmmConsumer* ommConsumer,
	rtsdk::ema::access::OmmConsumerClient* client) :
AppThread(ommConsumer),
 _reqMsg(),
_client(client)
{
}

ConsumerThread::~ConsumerThread()
{
}

void ConsumerThread::openItem(const rtsdk::ema::access::EmaString& item, 
							 const rtsdk::ema::access::EmaString& serviceName,
	                         unsigned int mode)
{
	_itemNamePrefix = item;
	_serviceName = serviceName;
	_mode = mode;
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
		
			if (_mode == 1)
			{
				// STREAMING VIEW 1
				_reqMsg.clear();
				_reqMsg.payload(ElementList().addUInt(ENAME_VIEW_TYPE, 1).addArray(ENAME_VIEW_DATA, OmmArray().fixedWidth(2).addInt(22).addInt(25).complete()).complete());
				_reqMsg.name(itemName).serviceName(_serviceName).interestAfterRefresh(true);
			}
			else if (_mode == 2)
			{
				// SNAPSHOT VIEW 2
				_reqMsg.clear();
				_reqMsg.payload(ElementList().addUInt(ENAME_VIEW_TYPE, 1).addArray(ENAME_VIEW_DATA, OmmArray()
					.addInt(2).addInt(3).addInt(4).addInt(5).addInt(6).addInt(7)
					.addInt(8).addInt(9).addInt(10).addInt(11).addInt(12).addInt(13)
					.addInt(14).addInt(15).addInt(16).addInt(18).addInt(19)
					.addInt(21).addInt(23).addInt(24).addInt(25).addInt(26).addInt(27)
					.addInt(28).addInt(29).addInt(30).addInt(31).addInt(32).addInt(33)
					.complete()).complete());
				_reqMsg.name(itemName).serviceName(_serviceName).interestAfterRefresh(false);
			}
			else if (_mode == 3)
			{
				// BATCH STREAMING REQUESTS
				_reqMsg.clear();
				_reqMsg.payload(ElementList().addArray(ENAME_BATCH_ITEM_LIST, OmmArray().addAscii("BATCHITEM1").addAscii("BATCHITEM2").complete()).complete());
				_reqMsg.serviceName(_serviceName).interestAfterRefresh(false);

				++ResultValidation::_numRequestOpen;
			}

			ResultValidation* closure = new ResultValidation(itemName);
			_ommConsumer->registerClient(_reqMsg, *_client, (void *)(closure));
			ResultValidation::closuresList.push_back((void *)closure);

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
