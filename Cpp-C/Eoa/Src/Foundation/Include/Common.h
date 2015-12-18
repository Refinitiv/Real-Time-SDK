/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_common_h_
#define __thomsonreuters_eoa_foundation_common_h_

#pragma warning( disable : 4290 )

/**
	@file Common.h "Foundation/Include/Common.h"
	@brief Common.h file provides definitions of common variable types used in EOA.
*/

namespace thomsonreuters {

namespace eoa {

/**
	@namespace foundation
	@brief The foundation namespace defines all the interfaces used by the EOA
	Foundation and Domain packages as well as applications using these packages.
	
	The EOA Foundation Layer provides base EOA services and functionalities.
*/
namespace foundation {

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

}

/**
	@namespace domain
	@brief The domain namespace defines all the interfaces used by the EOA
	Domain package as well as applications using this package.
*/
namespace domain {
}

}

}

#if defined( WIN32 ) && !defined( __EOA_STATIC_BUILD__ )
	#ifdef EOA_LIBRARY_EXPORTS 
		#define EOA_FOUNDATION_API		__declspec(dllexport)
		#define EOA_DOMAIN_API			__declspec(dllexport)
	#else
		#define EOA_FOUNDATION_API		__declspec(dllimport)
		#define EOA_DOMAIN_API			__declspec(dllimport)
	#endif
#else 
	#define EOA_FOUNDATION_API
	#define EOA_DOMAIN_API
#endif

/*!
    \page Using EOA in Multi-Threaded Applications

	By providing thread safe and thread aware implementation, EOA enables application
	developers to take full advantage of multi-core processors and their scalability.
	On the same token, EOA enables developers to design multi-threaded applications that
	best meet their business needs. While designing a multi-threaded EOA application one
	must consider concurrent accessibility of EOA methods from different threads of control.
	The following information is used to identify this capability of each EOA method.

    Depending on whether or not a concurrent access to EOA method is allowed and how,
	EOA methods may be grouped into three categories:

    \section ObjectLevelSafe Object Level Thread Safe
	Multiple application threads of control may concurrently call this method on the same
	instance. Applications will, however, incur the cost of synchronization overhead
	(e.g., blocking) while calling this method. By synchronizing concurrent access
	to this method on per instance basis, EOA preserves the state of individual instances
	and resolves any race condition that may arise while multiple application threads of
	control concurrently call this method on the same instance.

    \section ClassLevelSafe Class Level Thread Safe
	Multiple application threads of control may concurrently call this method across all
	instances. Applications will, however, incur the cost of synchronization overhead
	(e.g., blocking) while calling this method. By synchronizing concurrent access to this
	method on per class basis, EOA preserves the state of all instances and resolves any
	race condition that may arise while multiple application threads of control concurrently
	call this method.

    \section SingleThreaded Lock-free
	Single application thread of control may call such methods on an instance incurring no 
	cost of synchronization overhead. EOA does not synchronize access to this method.
	Concurrent access to the same instance using this method may result in an undefined behaviour.

    EOA method that is considered a thread safe method is explicitly and
	respectively marked as such in this Reference Manual. If there is no thread safety
	marking on a method, this method is considered a \ref SingleThreaded.

    Please see EOA Developer Guide for more details.
*/

#endif // __thomsonreuters_eoa_foundation_common_h_
