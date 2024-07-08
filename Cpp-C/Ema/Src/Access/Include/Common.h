/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_Common_h_
#define __refinitiv_ema_access_Common_h_

/**
	@file Common.h "Access/Include/Common.h"
	@brief Common.h file provides definitions of common variable types used in EMA.
*/

namespace refinitiv {

namespace ema {

/**
	@namespace access
	@brief The access namespace contains all interfaces and definitions specified
			for use with the EMA Access package.
*/
namespace access {

/**
	\typedef UInt8
	@brief represents 8-bit unsigned integer
*/
typedef unsigned char UInt8;

/**
	\typedef Int8
	@brief represents 8-bit signed integer
*/
typedef signed char Int8;

/**
	\typedef UInt16
	@brief represents 16-bit unsigned integer
*/
typedef unsigned short UInt16;

/**
	\typedef Int16
	@brief represents 16-bit signed integer
*/
typedef short Int16;

/**
	\typedef UInt32
	@brief represents 32-bit unsigned integer
*/
typedef unsigned int UInt32;

/**
	\typedef Int32
	@brief represents 32-bit signed integer
*/
typedef int Int32;

/**
	\typedef UInt64
	@brief represents 64-bit unsigned integer
*/
typedef unsigned long long UInt64;

/**
	\typedef Int64
	@brief represents 64-bit signed integer
*/
typedef long long Int64;

/**
	\typedef Double
	@brief represents 8-byte floating point value
*/
typedef double  Double;

}

}

}

#if defined( WIN32 ) && !defined( __EMA_STATIC_BUILD__ )
	#ifdef EMA_LIBRARY_EXPORTS 
		#define EMA_ACCESS_API		__declspec(dllexport)
	#else
		#define EMA_ACCESS_API		__declspec(dllimport)
	#endif
#else 
	#define EMA_ACCESS_API
#endif

/*!
    \page Using EMA in Multi-Threaded Applications

	By providing thread safe and thread aware implementation, EMA enables application
	developers to take full advantage of multi-core processors and their scalability.
	On the same token, EMA enables developers to design multi-threaded applications that
	best meet their business needs. While designing a multi-threaded EMA application one
	must consider concurrent accessibility of EMA methods from different threads of control.
	The following information is used to identify this capability of each EMA method.

    Depending on whether or not a concurrent access to EMA method is allowed and how,
	EMA methods may be grouped into three categories:

    \section ObjectLevelSafe Object Level Thread Safe Method
	Multiple application threads of control may concurrently call this method on the same
	instance. Applications will, however, incur the cost of synchronization overhead
	(e.g., blocking) while calling this method. By synchronizing concurrent access
	to this method on per instance basis, EMA preserves the state of individual instances
	and resolves any race condition that may arise while multiple application threads of
	control concurrently call this method on the same instance.

    \section ClassLevelSafe Class Level Thread Safe Method
	Multiple application threads of control may concurrently call this method across all
	instances. Applications will, however, incur the cost of synchronization overhead
	(e.g., blocking) while calling this method. By synchronizing concurrent access to this
	method on per class basis, EMA preserves the state of all instances and resolves any
	race condition that may arise while multiple application threads of control concurrently
	call this method.

    \section SingleThreaded Lock-free Method
	Single application thread of control may call such methods on an instance incurring no 
	cost of synchronization overhead. EMA does not synchronize access to this method.
	Concurrent access to the same instance using this method may result in an undefined behaviour.

    EMA method that is considered a thread safe method is explicitly and
	respectively marked as such in this Reference Manual. If there is no thread safety
	marking on a method, this method is considered a \ref SingleThreaded.

    Please see EMA Developer Guide for more details.
*/

#endif // __refinitiv_ema_access_Common_h_
