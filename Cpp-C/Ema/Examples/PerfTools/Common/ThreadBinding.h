///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __ema__threadBinding_h__
#define __ema__threadBinding_h__

#include "Ema.h"

// the following methods are declared as public interfaces for applications to:
// - explicitly bind application threads
// - implicitly bind EMA threads


// this method is used to set affinity of the thread whose thread id is passed in
//
// note if passed in threadId is set to zero, the calling thread will be bound
//
// note that this method MUST be called in the beginning of the thread::run()
// or main() to make sure that all thread activities are on the "bound" thread
//
extern void bindThisThread( const rtsdk::ema::access::EmaString& threadName, long cpu, long threadId = 0 );

// these methods take snapshots of all the threads running in the calling process
// at the moment of calling the methods
// 
// these methods allow to infer the id of a new thread(s) that showed up in the calling process
// from the moment firstThreadSnapshot() was called to the moment secondThreadSnapshot() was called
//
// note that functionality of those two methods heavilly depends on the inference that the new thread(s)
// belong to EMA and are started by EMA due to specific application events
// ( or belong to the application and are started by the application )
//
// if application creates threads at the time those methods are called, there is a chance 
// a new app thread will be treated as an EMA internal thread
//
// note that those two methods must work together since they both lock / unlock internal resources
//
extern void firstThreadSnapshot();
extern void secondThreadSnapshot( const rtsdk::ema::access::EmaString& threadName, long cpu );

// method to print out information about threads that are currently bound
//
extern void printAllThreadBinding();

#endif // __ema__threadBinding_h__
