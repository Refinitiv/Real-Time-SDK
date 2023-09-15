/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|          Copyright (C) 2019-2022 Refinitiv. All rights reserved.          --
*|-----------------------------------------------------------------------------
*/

#ifndef __refinitiv_ema_access_OmmBaseImplMap_h
#define __refinitiv_ema_access_OmmBaseImplMap_h

#include "EmaVector.h"
#include "Mutex.h"
#include "ActiveConfig.h"
#include "GlobalPool.h"

#ifdef WIN32
#include <windows.h>
#else
#include <assert.h>
#include <sys/time.h>
#include <stdio.h>
#endif

#ifdef WIN32
#define USING_SELECT
#else
#define USING_POLL
#define USING_PPOLL
#endif

#ifdef USING_PPOLL
#include <poll.h>
#endif

namespace refinitiv {

namespace ema {

namespace access {

class OmmLoggerClient;
class ErrorClientHandler;

class OmmCommonImpl
{
public:
		OmmCommonImpl() {
#ifdef USING_POLL
			_eventFds = NULL;
			_eventFdsCount = 0;
			_eventFdsCapacity = 0;
			_pipeReadEventFdsIdx = -1;
#endif
		}

		enum ImplementationType
		{
			ConsumerEnum,
			NiProviderEnum,
			IProviderEnum
		};

		virtual ImplementationType getImplType() = 0;

		virtual void handleIue(const EmaString&, Int32 errorCode) = 0;

		virtual void handleIue(const char*, Int32 errorCode) = 0;

		virtual void handleIhe(UInt64, const EmaString&) = 0;

		virtual void handleIhe(UInt64, const char*) = 0;

		virtual void handleMee(const char*) = 0;

		virtual LoggerConfig& getActiveLoggerConfig() = 0;

		virtual OmmLoggerClient& getOmmLoggerClient() = 0;

		virtual ErrorClientHandler& getErrorClientHandler() = 0;

		virtual bool hasErrorClientHandler() const = 0;

		virtual const EmaString& getInstanceName() const = 0;

		virtual void msgDispatched(bool value = true) = 0;

		virtual Mutex& getUserMutex() = 0;

		virtual bool isAtExit() = 0;

#ifdef USING_POLL
  void removeFd( int );
  int addFd( int, short events = POLLIN );
  void removeAllFd();
#endif

protected:
#ifdef USING_POLL
  pollfd*			_eventFds;
  nfds_t			_eventFdsCount;
  nfds_t			_eventFdsCapacity;
  int				_pipeReadEventFdsIdx;
#endif

};

template <class T> class OmmBaseImplMap
{
public:

	static UInt64 add(T* impl)
	{
		_listLock.lock();
		if (_clientList.empty())
			OmmBaseImplMap<T>::init();

		_clientList.push_back(impl);
		++_id;

		_listLock.unlock();

		return _id;
	}

	static void remove(T* impl)
	{
		if (_clearSigHandler)
		{
			return;
		}

		_listLock.lock();

		_clientList.removeValue(impl);

		if (!_clientList.empty() || _clearSigHandler)
		{
			_listLock.unlock();
			return;
		}

#ifdef WIN32
		SetConsoleCtrlHandler(&OmmBaseImplMap<T>::TermHandlerRoutine, FALSE);
#else
		sigaction(SIGINT, &_oldSigAction, NULL);
#endif

		_clearSigHandler = true;

		_listLock.unlock();

#ifndef WIN32
		notifyMonitorThread();
#endif
	}

	static bool has(T* impl)
	{
		bool hasObject = false;

		_listLock.lock();

		hasObject = _clientList.getPositionOf(impl) != -1 ? true : false;

		_listLock.unlock();

		return hasObject;
	}

	static void acquireCleanupLock()
	{
		_cleanupLock.lock();
	}

	static void releaseCleanupLock()
	{
		_cleanupLock.unlock();
	}

#ifdef WIN32
	static BOOL WINAPI TermHandlerRoutine(DWORD dwCtrlType)
	{
		switch (dwCtrlType)
		{
		case CTRL_CLOSE_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_SHUTDOWN_EVENT:
		case CTRL_C_EVENT:
			_CtrlC_Activated = true;
			OmmBaseImplMap<T>::atExit();
			break;
		}
		return FALSE;
	}
#else
	static void* runExitThread(void* arg)
	{
		OmmBaseImplMap<T>::atExit();
		exit(-1);
	}

	static void notifyMonitorThread()
	{
		_condVariableMutex.lock();
		_condVariable.notify();
		_condVariableMutex.unlock();
	}

	static void sigAction(int sig, siginfo_t* pSiginfo, void* pv)
	{
		_CtrlC_Activated = true;
		notifyMonitorThread();
	}

	static void* monitorThread(void* arg)
	{
		_condVariableMutex.lock();

		while (!_CtrlC_Activated && !_clearSigHandler)
		{
			_condVariable.wait(_condVariableMutex);
		}

		_condVariableMutex.unlock();

		if (_CtrlC_Activated)
			runExitThread(NULL);

		_monitorThreadMutex.lock();
		_monitorThreadId = 0;
		_monitorThreadMutex.unlock();

		return (void*)NULL;
	}

	static void runMonitorThread()
	{
		if (_monitorThreadId == 0)
		{
			_monitorThreadMutex.lock();
			if (_monitorThreadId == 0)
			{
				pthread_create(&_monitorThreadId, NULL, OmmBaseImplMap<T>::monitorThread, NULL);
				assert(_monitorThreadId != 0);
			}
			_monitorThreadMutex.unlock();
		}
		return;
	}
#endif

	static void sleep(UInt32 millisecs)
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

private:

	static void init()
	{
		_clearSigHandler = false;
#ifdef WIN32
		SetConsoleCtrlHandler(&OmmBaseImplMap<T>::TermHandlerRoutine, TRUE);
#else
		runMonitorThread();

		bzero(&_sigAction, sizeof(_sigAction));
		bzero(&_oldSigAction, sizeof(_oldSigAction));

		_sigAction.sa_sigaction = sigAction;
		_sigAction.sa_flags = SA_SIGINFO;

		sigaction(SIGINT, &_sigAction, &_oldSigAction);
#endif
	}

	static void atExit()
	{
		// set the global pool state as final
		// stop all operations with the global pool
		refinitiv::ema::access::GlobalPool::setFinalState();

		_cleanupLock.lock();
		_listLock.lock();

		UInt32 size = _clientList.size();
		EmaVector< T* > copyClientList(size);

		while (size)
		{
			T* pTemp = _clientList[size - 1];

			// We should remove console handler from the exit thread - therefore prevent invoke to remove from any app thread
			// Windows will deadlock when console handler is active and the app thread trying to remove it
			// We can invoke SetConsoleCtrlHandler from the app thread when the handler is inactive only
			OmmBaseImplMap<T>::remove(pTemp);
			size = _clientList.size();

			pTemp->setAtExit();

			copyClientList.push_back(pTemp);
		}

		_listLock.unlock();

		OmmBaseImplMap<T>::sleep(500);

		size = copyClientList.size();
		for (UInt32 i = 0; i < size; ++i)
		{
			T* pTemp = copyClientList[i];
			pTemp->uninitialize(false, false);
		}

		_cleanupLock.unlock();

		OmmBaseImplMap<T>::sleep(100);
		return;
	}

	static Mutex						_listLock;
	static Mutex						_cleanupLock;
	static EmaVector< T* >				_clientList;
	static UInt64						_id;
	static bool							_clearSigHandler;
	static bool							_CtrlC_Activated;

#ifndef WIN32
	static struct sigaction			_sigAction;
	static struct sigaction			_oldSigAction;

	static ConditionVariable		_condVariable;
	static Mutex					_condVariableMutex;

	static pthread_t				_monitorThreadId;
	static Mutex					_monitorThreadMutex;
#endif
};

template <class T> EmaVector< T* > OmmBaseImplMap<T>::_clientList;
template <class T> Mutex OmmBaseImplMap<T>::_listLock;
template <class T> Mutex OmmBaseImplMap<T>::_cleanupLock;
template <class T> UInt64 OmmBaseImplMap<T>::_id = 0;
template <class T> bool OmmBaseImplMap<T>::_clearSigHandler = true;
template <class T> bool OmmBaseImplMap<T>::_CtrlC_Activated = false;

#ifndef WIN32
template <class T> struct sigaction OmmBaseImplMap<T>::_sigAction;
template <class T> struct sigaction OmmBaseImplMap<T>::_oldSigAction;

template <class T> ConditionVariable OmmBaseImplMap<T>::_condVariable;
template <class T> Mutex OmmBaseImplMap<T>::_condVariableMutex;

template <class T> pthread_t OmmBaseImplMap<T>::_monitorThreadId = 0;
template <class T> Mutex OmmBaseImplMap<T>::_monitorThreadMutex;
#endif

}

}

}

#endif // __refinitiv_ema_access_OmmBaseImplMap_h

