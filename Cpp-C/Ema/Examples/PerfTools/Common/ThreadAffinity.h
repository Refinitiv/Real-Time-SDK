///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|          Copyright (C) 2019-2022 LSEG. All rights reserved.               --
///*|-----------------------------------------------------------------------------

#ifndef __ema__threadAffinity_h__
#define __ema__threadAffinity_h__

#include "Ema.h"

// class associating info about particular threads;
// this is used for printout and determination of thread affinity binding
//
class ThreadInfo
{
public:

	ThreadInfo() : _threadId( 0 ), _requestedCPU( 0 ), _boundCPU( 0 ) {}
	virtual ~ThreadInfo() {}

	void setThreadId( long id ) { _threadId = id; }
	void setThreadName( const refinitiv::ema::access::EmaString& name ) { _threadName = name; }
	void setRequestedCPU( long cpu ) { _requestedCPU = cpu; }
	void setBoundCPU( long cpu ) { _boundCPU.append(static_cast<int>(cpu)); }
	void setBoundCPU( const refinitiv::ema::access::EmaString& cpuString ) { _boundCPU = cpuString; }

	long getThreadId() const { return _threadId; }
	const refinitiv::ema::access::EmaString& getThreadName() const { return _threadName; }
	long getRequestedCPU() const { return _requestedCPU; }
	//long getBoundCPU() const { return _boundCPU; }
	const refinitiv::ema::access::EmaString& getBoundCPU() const { return _boundCPU; }

	ThreadInfo& operator=( const ThreadInfo& );
	ThreadInfo( const ThreadInfo& );
	bool operator==( const ThreadInfo& ) const;

private:

	long										_threadId;
	long										_requestedCPU;
	//long										_boundCPU;
	refinitiv::ema::access::EmaString			_boundCPU;
	refinitiv::ema::access::EmaString		_threadName;
};

#endif // __ema__threadAffinity_h__
