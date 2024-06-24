/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef RSSL_NOTIFIER_H
#define RSSL_NOTIFIER_H

 /* Cross-platform notification.
  * 
  * Overview of usage:
  * - rsslCreateNotifier creates an RsslNotifier that can wait on file descriptors.
  * - rsslCreateNotifierEvent creates an RsslNotifierEvent, associated with a file descriptor.
  * - rsslNotifierAddEvent adds the RsslNotifierEvent to the RsslNotifier.
  * - rsslNotifierRegisterRead/rsslNotifierRegisterWrite enable read/write notification for the RsslNotifierEvent.
  * - rsslNotifierWait waits for notification on the RsslNotifierEvents associated with the RsslNotifier, for the specified time.
  *     The triggered events can be found in the RsslNotifier.notifiedEvents array. 
  *     They will remain there till the next call to rsslNotifierWait.
  * - rsslNotifierUnregisterRead/rsslNotifierUnregisterWrite disable read/write notification for the RsslNotifierEvent.
  * - rsslNotifierRemoveEvent removes an RsslNotifierEvent from the RsslNotifier.
  * - rsslDestroyNotifierEvent cleans up an RsslNotifierEvent.
  * - rsslDestroyNotifier cleans up the RsslNotifier. */

#include "rtr/os.h"
#include "rtr/rsslVAExports.h"
#include "rtr/rsslTransport.h"

#if defined(WIN32)
#elif defined(Linux)
#include <sys/epoll.h>
#define RSSL_NOTIFIER_EPOLL
#else
#include <sys/select.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Flags for notifier events. */
typedef enum
{
	RSSL_NESF_READ = 0x1,	/* Indicates file can be read from without blocking. */
	RSSL_NESF_WRITE = 0x2,	/* Indicates file can be written to without blocking. */
	RSSL_NESF_BAD_FD = 0x4  /* Indicates descriptor may be invalid. */
} RsslNotifierEventFlags;

typedef struct 
{
	int notifiedFlags; /* Flags indicating whether read or write would block. See RsslNotifierEventFlags. */
} RsslNotifierEvent;

/* Indicates whether the event's file can be read from without blocking. */
RTR_C_INLINE int rsslNotifierEventIsReadable(RsslNotifierEvent *pEvent)
{
	return pEvent->notifiedFlags & RSSL_NESF_READ;
}

/* Indicates whether the event's file can be written to without blocking. */
RTR_C_INLINE int rsslNotifierEventIsWritable(RsslNotifierEvent *pEvent)
{
	return pEvent->notifiedFlags & RSSL_NESF_WRITE;
}

/* Indicates whether the event's file descriptor may be invalid.  The event may need its associated FD to be updated.
 *   Note: When the notifier uses select for notification, this will be set on every descriptor when
 *   it sees the EBADF error. When the notifier uses poll, it will be set only on appropriate events. */
RTR_C_INLINE int rsslNotifierEventIsFdBad(RsslNotifierEvent *pEvent)
{
	return pEvent->notifiedFlags & RSSL_NESF_BAD_FD;
}

/* Clears the event's notification flags. May be useful if you are done reading from/writing to a notified event. */
RTR_C_INLINE void rsslNotifierEventClearNotifiedFlags(RsslNotifierEvent *pEvent)
{
	pEvent->notifiedFlags = 0;
}

/* Creates an RsslNotifierEvent. */
RSSL_API RsslNotifierEvent *rsslCreateNotifierEvent();

/* Destroys an RsslNotifierEvent. */
RSSL_API void rsslDestroyNotifierEvent(RsslNotifierEvent *pEvent);

/* Returns the object associated with this event. */
RSSL_API void *rsslNotifierEventGetObject(RsslNotifierEvent *pEvent);

/* Used to wait for notification.
 * Triggers on any of the RsslNotifierEvents associated with it. */
typedef struct
{
	RsslNotifierEvent	**notifiedEvents;
	int					notifiedEventCount;
} RsslNotifier;

/* Initializes an RsslNotifier. 
 * - maxEventsHint: The likely max number of associated events. Setting appropriately may improve performance. */
RSSL_API RsslNotifier *rsslCreateNotifier(int maxEventsHint);

/* Cleans up resources associated with an RsslNotifier. */
RSSL_API void rsslDestroyNotifier(RsslNotifier *pNotifier);

/* Adds an RsslNotifierEvent to the RsslNotifier. */
RSSL_API int rsslNotifierAddEvent(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent, RsslSocket fd, void *object);

/* Removes an RsslNotifierEvent from the RsslNotifier. */
RSSL_API int rsslNotifierRemoveEvent(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent);

/* Updates the descriptor associated with an event.  The previous descriptor must already be closed. */
RSSL_API int rsslNotifierUpdateEventFd(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent, RsslSocket fd);

/* Registers the RsslNotifierEvent's file for read (and out-of-band) notfication. */
RSSL_API int rsslNotifierRegisterRead(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent);

/* Unregisters the RsslNotifierEvent's file for read (and out-of-band) notfication. */
RSSL_API int rsslNotifierUnregisterRead(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent);

/* Registers the RsslNotifierEvent's file for write  notfication. */
RSSL_API int rsslNotifierRegisterWrite(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent);

/* Unregisters the RsslNotifierEvent's file for write  notfication. */
RSSL_API int rsslNotifierUnregisterWrite(RsslNotifier *pNotifier, RsslNotifierEvent *pEvent);

/* Wait for notifcation on the notifier's associated files. */
RSSL_API int rsslNotifierWait(RsslNotifier *pNotifier, long timeoutUsec);

#ifdef __cplusplus
}
#endif

#endif
