/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslNotifier.h"
#include <stdlib.h>

/* On windows, select is used for notification.
 * Otherwise poll is used. */
#if defined(WIN32)
#define FD_SETSIZE 6400
#include <winsock2.h>
#else
#include <poll.h>
#endif

typedef struct
{
	RsslNotifierEvent base;

	int _registeredFlags; /* RsslNotifierEvent flags set on this event. */
	void *_object;

#ifndef WIN32
	int _pollFdIndex; /* Array index of the pollfd associated with this event */
#else
	SOCKET _fd;
#endif

} RsslNotifierEventImpl;

typedef struct
{
	RsslNotifier base;
	RsslNotifierEventImpl **_events; /* RsslNotifierEvents associated with this notifier */
	int _maxEvents; /* Maximum number of events the array can currently hold */
	int _eventCount; /* Number of events in the array */
#ifndef WIN32
	struct pollfd *_pollFds; /* Array of pollfds associated with events in this notifier. */
#else
	fd_set _readFds; /* Read fd_set */
	fd_set _writeFds; /* Write fd_set */
	fd_set _exceptFds; /* Except fd_set */
#endif
} RsslNotifierImpl;

RSSL_API RsslNotifierEvent *rsslCreateNotifierEvent()
{
	return (RsslNotifierEvent*)calloc(sizeof(RsslNotifierEventImpl), 1);
}

RSSL_API void rsslDestroyNotifierEvent(RsslNotifierEvent *pEvent)
{
	free(pEvent);
}

RSSL_API void *rsslNotifierEventGetObject(RsslNotifierEvent *pEvent)
{
	RsslNotifierEventImpl *pNotifierEventImpl = (RsslNotifierEventImpl*)pEvent;
	return pNotifierEventImpl->_object;
}


RSSL_API RsslNotifier *rsslCreateNotifier(int maxEventsHint)
{
	RsslNotifierImpl *pNotifierImpl = (RsslNotifierImpl*)malloc(sizeof(RsslNotifierImpl));
	memset(pNotifierImpl, 0, sizeof(RsslNotifierImpl));

	pNotifierImpl->_maxEvents = maxEventsHint;
	pNotifierImpl->_events = (RsslNotifierEventImpl**)malloc(maxEventsHint * sizeof(RsslNotifierEventImpl**));
	if (pNotifierImpl->_events == NULL)
	{
		rsslDestroyNotifier(&pNotifierImpl->base);
		return NULL;
	}

	pNotifierImpl->base.notifiedEvents = (RsslNotifierEvent**)malloc(maxEventsHint * sizeof(RsslNotifierEvent**));
	if (pNotifierImpl->base.notifiedEvents == NULL)
	{
		rsslDestroyNotifier(&pNotifierImpl->base);
		return NULL;
	}

#ifndef WIN32
	pNotifierImpl->_pollFds = (struct pollfd*)malloc(maxEventsHint * sizeof(struct pollfd));
	if (pNotifierImpl->_pollFds == NULL)
	{
		rsslDestroyNotifier(&pNotifierImpl->base);
		return NULL;
	}
#endif

	return &pNotifierImpl->base;
}

RSSL_API void rsslDestroyNotifier(RsslNotifier *pNotifier)
{
	RsslNotifierImpl *pNotifierImpl = (RsslNotifierImpl*)pNotifier;
	free(pNotifierImpl->_events);
	pNotifierImpl->_events = NULL;

	free(pNotifierImpl->base.notifiedEvents);
	pNotifierImpl->base.notifiedEvents = NULL;

#ifndef WIN32
	free(pNotifierImpl->_pollFds);
	pNotifierImpl->_pollFds = NULL;
#endif

	free(pNotifierImpl);
}

RSSL_API int rsslNotifierAddEvent(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent, RsslSocket fd, void *object)
{
	RsslNotifierImpl *pNotifierImpl = (RsslNotifierImpl*)pNotifier;
	RsslNotifierEventImpl *pNotifierEventImpl = (RsslNotifierEventImpl*)pEvent;

#ifdef WIN32
	/* Since we are directly setting the fd_sets on Windows, make sure we don't overrun the fd_array. */
	if (pNotifierImpl->_eventCount == FD_SETSIZE)
		return -1;
#endif


	if (pNotifierImpl->_eventCount == pNotifierImpl->_maxEvents)
	{
		/* Event arrays are full; double their sizes before adding the new event. */
#ifndef WIN32
		struct pollfd *pollFds;
#endif
		RsslNotifierEvent **notifiedEvents;
		RsslNotifierEventImpl **events = (RsslNotifierEventImpl**)realloc(pNotifierImpl->_events, pNotifierImpl->_maxEvents * 2 * sizeof(RsslNotifierEventImpl**));
		if (events == NULL)
			return -1;

		notifiedEvents = (RsslNotifierEvent**)realloc(pNotifierImpl->base.notifiedEvents, pNotifierImpl->_maxEvents * 2 * sizeof(RsslNotifierEvent**));
		if (notifiedEvents == NULL)
			return -1;

#ifndef WIN32
		pollFds = (struct pollfd*)realloc(pNotifierImpl->_pollFds, pNotifierImpl->_maxEvents * 2 * sizeof(struct pollfd));
		if (pollFds == NULL)
			return -1;
#endif

		pNotifierImpl->_maxEvents *= 2;
		pNotifierImpl->_events = events;
		pNotifierImpl->base.notifiedEvents = notifiedEvents;
#ifndef WIN32
		pNotifierImpl->_pollFds = pollFds;
#endif
	}

#ifndef WIN32
	memset(&pNotifierImpl->_pollFds[pNotifierImpl->_eventCount], 0, sizeof(struct pollfd));
	pNotifierImpl->_pollFds[pNotifierImpl->_eventCount].fd = fd;
	pNotifierEventImpl->_pollFdIndex = pNotifierImpl->_eventCount;
#else
	pNotifierEventImpl->_fd = fd;
#endif

	pNotifierEventImpl->_object = object;

	pNotifierImpl->_events[pNotifierImpl->_eventCount] = pNotifierEventImpl;
	++pNotifierImpl->_eventCount;
	return 0;
}

RSSL_API int rsslNotifierUpdateEventFd(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent, RsslSocket fd)
{
	RsslNotifierImpl *pNotifierImpl = (RsslNotifierImpl*)pNotifier;
	RsslNotifierEventImpl *pNotifierEventImpl = (RsslNotifierEventImpl*)pEvent;
	int i;

	pNotifierEventImpl->_registeredFlags = 0;

	for (i = 0; i < pNotifierImpl->_eventCount; ++i)
	{
		if (pNotifierImpl->_events[i] == pNotifierEventImpl)
		{

#ifndef WIN32
			pNotifierImpl->_pollFds[pNotifierEventImpl->_pollFdIndex].fd = fd;
#else
			pNotifierEventImpl->_fd = fd;
#endif
			return 0;
		}
	}

	return -1; /* Not found. */

}

RSSL_API int rsslNotifierRemoveEvent(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent)
{
	RsslNotifierImpl *pNotifierImpl = (RsslNotifierImpl*)pNotifier;
	RsslNotifierEventImpl *pNotifierEventImpl = (RsslNotifierEventImpl*)pEvent;
	int i;

	pNotifierEventImpl->_registeredFlags = 0;

	for (i = 0; i < pNotifierImpl->_eventCount; ++i)
	{
		if (pNotifierImpl->_events[i] == pNotifierEventImpl)
		{
			/* Swap in last event */
			if (pNotifierImpl->_eventCount > 1)
			{
#ifndef WIN32
				pNotifierImpl->_events[pNotifierImpl->_eventCount - 1]->_pollFdIndex = i;
				pNotifierImpl->_pollFds[i] = pNotifierImpl->_pollFds[pNotifierImpl->_eventCount - 1];
#endif

				pNotifierImpl->_events[i] = pNotifierImpl->_events[pNotifierImpl->_eventCount - 1];
			}

			--pNotifierImpl->_eventCount;
			break;
		}
	}

	return 0;

}

RSSL_API int rsslNotifierRegisterRead(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent)
{
	RsslNotifierImpl *pNotifierImpl = (RsslNotifierImpl*)pNotifier;
	RsslNotifierEventImpl *pNotifierEventImpl = (RsslNotifierEventImpl*)pEvent;
	pNotifierEventImpl->_registeredFlags |= RSSL_NESF_READ;
#ifndef WIN32
	pNotifierImpl->_pollFds[pNotifierEventImpl->_pollFdIndex].events |= POLLIN | POLLPRI;
#endif
	return 0;
}

RSSL_API int rsslNotifierUnregisterRead(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent)
{
	RsslNotifierImpl *pNotifierImpl = (RsslNotifierImpl*)pNotifier;
	RsslNotifierEventImpl *pNotifierEventImpl = (RsslNotifierEventImpl*)pEvent;
	pNotifierEventImpl->_registeredFlags &= ~RSSL_NESF_READ;
#ifndef WIN32
	pNotifierImpl->_pollFds[pNotifierEventImpl->_pollFdIndex].events &= ~(POLLIN | POLLPRI);
#endif
	return 0;
}


RSSL_API int rsslNotifierRegisterWrite(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent)
{
	RsslNotifierImpl *pNotifierImpl = (RsslNotifierImpl*)pNotifier;
	RsslNotifierEventImpl *pNotifierEventImpl = (RsslNotifierEventImpl*)pEvent;
	pNotifierEventImpl->_registeredFlags |= RSSL_NESF_WRITE;
#ifndef WIN32
	pNotifierImpl->_pollFds[pNotifierEventImpl->_pollFdIndex].events |= POLLOUT;
#endif
	return 0;
}

RSSL_API int rsslNotifierUnregisterWrite(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent)
{
	RsslNotifierImpl *pNotifierImpl = (RsslNotifierImpl*)pNotifier;
	RsslNotifierEventImpl *pNotifierEventImpl = (RsslNotifierEventImpl*)pEvent;
	pNotifierEventImpl->_registeredFlags &= ~RSSL_NESF_WRITE;
#ifndef WIN32
	pNotifierImpl->_pollFds[pNotifierEventImpl->_pollFdIndex].events &= ~POLLOUT;
#endif
	return 0;
}

RSSL_API int rsslNotifierWait(RsslNotifier *pNotifier, long timeoutUsec)
{
	RsslNotifierImpl *pNotifierImpl = (RsslNotifierImpl*)pNotifier;
	int i;
	int ret;

#ifndef WIN32
	pNotifierImpl->base.notifiedEventCount = 0;
	ret = poll(pNotifierImpl->_pollFds, pNotifierImpl->_eventCount, timeoutUsec/1000);
	if (ret < 0)
		return ret;

	for (i = 0; i < pNotifierImpl->_eventCount; ++i)
	{
		RsslNotifierEventImpl *pNotifierEventImpl = pNotifierImpl->_events[i];
		pNotifierEventImpl->base.notifiedFlags = 0;

		if (pNotifierImpl->_pollFds[i].revents & (POLLIN | POLLPRI))
			pNotifierEventImpl->base.notifiedFlags |= RSSL_NESF_READ;

		if (pNotifierImpl->_pollFds[i].revents & POLLOUT)
			pNotifierEventImpl->base.notifiedFlags |= RSSL_NESF_WRITE;

		if (pNotifierImpl->_pollFds[i].revents & POLLNVAL)
			pNotifierEventImpl->base.notifiedFlags |= RSSL_NESF_BAD_FD;

		/* If notified, add to list. */
		if (pNotifierImpl->_events[i]->base.notifiedFlags)
		{
			pNotifierImpl->base.notifiedEvents[pNotifierImpl->base.notifiedEventCount] = &pNotifierImpl->_events[i]->base;
			++pNotifierImpl->base.notifiedEventCount;
		}
	}

#else

	struct timeval selectTime;

	FD_ZERO(&pNotifierImpl->_readFds);
	FD_ZERO(&pNotifierImpl->_writeFds);
	FD_ZERO(&pNotifierImpl->_exceptFds);

	for (i = 0; i < pNotifierImpl->_eventCount; ++i)
	{
		RsslNotifierEventImpl *pNotifierEventImpl = pNotifierImpl->_events[i];
		pNotifierEventImpl->base.notifiedFlags = 0;

		/* On windows, set up the fd_sets directly. FD_SET takes a lot of time when there are many connections,
		 * since it's checking for duplicates. */
		if (pNotifierEventImpl->_registeredFlags & RSSL_NESF_READ)
		{
			pNotifierImpl->_readFds.fd_array[pNotifierImpl->_readFds.fd_count++] = pNotifierEventImpl->_fd;
			pNotifierImpl->_exceptFds.fd_array[pNotifierImpl->_exceptFds.fd_count++] = pNotifierEventImpl->_fd;
		}

		if (pNotifierEventImpl->_registeredFlags & RSSL_NESF_WRITE)
			pNotifierImpl->_writeFds.fd_array[pNotifierImpl->_writeFds.fd_count++] = pNotifierEventImpl->_fd;

	}

	/* Windows won't allow selecting if there are no descriptors in any set. Sleep for the specified time instead. */
	if (pNotifierImpl->_readFds.fd_count == 0 && pNotifierImpl->_writeFds.fd_count == 0 && pNotifierImpl->_exceptFds.fd_count == 0)
	{
		Sleep(timeoutUsec / 1000);
		return 0;
	}

	pNotifierImpl->base.notifiedEventCount = 0;
	selectTime.tv_sec = timeoutUsec / 1000000;
	selectTime.tv_usec = (timeoutUsec - (selectTime.tv_sec * 1000000));
	ret = select(FD_SETSIZE, &pNotifierImpl->_readFds, &pNotifierImpl->_writeFds, &pNotifierImpl->_exceptFds, &selectTime);
	if (ret < 0)
	{
		if (WSAGetLastError() == WSAENOTSOCK)
		{
			/* Select indicated we have a bad file descriptor, but don't know which one. Mark all file descriptors as possibly bad. */
			for (i = 0; i < pNotifierImpl->_eventCount; ++i)
			{
				pNotifierImpl->_events[i]->base.notifiedFlags = RSSL_NESF_BAD_FD;
				pNotifierImpl->base.notifiedEvents[pNotifierImpl->base.notifiedEventCount] = &pNotifierImpl->_events[i]->base;
				++pNotifierImpl->base.notifiedEventCount;
			}
			return pNotifierImpl->_eventCount;
		}
		else
			return ret;
	}
	else if (ret > 0)
	{
		for (i = 0; i < pNotifierImpl->_eventCount; ++i)
		{
			if (FD_ISSET(pNotifierImpl->_events[i]->_fd, &pNotifierImpl->_readFds) || FD_ISSET(pNotifierImpl->_events[i]->_fd, &pNotifierImpl->_exceptFds))
				pNotifierImpl->_events[i]->base.notifiedFlags |= RSSL_NESF_READ;

			if (FD_ISSET(pNotifierImpl->_events[i]->_fd, &pNotifierImpl->_writeFds))
				pNotifierImpl->_events[i]->base.notifiedFlags |= RSSL_NESF_WRITE;

			/* If notified, add to list. */
			if (pNotifierImpl->_events[i]->base.notifiedFlags)
			{
				pNotifierImpl->base.notifiedEvents[pNotifierImpl->base.notifiedEventCount] = &pNotifierImpl->_events[i]->base;
				++pNotifierImpl->base.notifiedEventCount;
			}
		}
	}

#endif
	return ret;
}
