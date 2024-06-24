///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#ifndef __ema_mutex_h_
#define __ema_mutex_h_

#include "Ema.h"

#if defined(WIN32)
#include "windows.h"
#else
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#endif

class Mutex
{
public:

	///@name Constructor
	//@{
		/** default constructor
		*/
		Mutex();
	//@}

	///@name Destructor
	//@{
		/** destructor
		*/
		virtual ~Mutex();
	//@}

	///@name Operations
	//@{
		/** method to obtain ownership to and lock the mutex variable
		*/
		void lock();

		/** method to release ownership of mutex variable
		*/
		void unlock();
	//@}

private:

	// data members
#if defined(WIN32)
	CRITICAL_SECTION		m_cs;
#else
	pthread_mutex_t			m_mutex;
	pthread_cond_t			m_cv;
#endif

	// not to be used methods
	Mutex( const Mutex & );
	Mutex & operator=( const Mutex & );
};

#endif // __ema_mutex_h_
