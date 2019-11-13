///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "ThreadAffinity.h"
#include "Mutex.h"
#include "AppUtil.h"

#include <iostream>
#include <list>

#ifdef WIN32

#include <windows.h>
#include <tlhelp32.h>

#else

#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#endif

using namespace thomsonreuters::ema::access;
using namespace perftool::common;
using namespace std;

Mutex		_collectionLock;

list< ThreadInfo >	_collection;		// contains all bound threads

list< unsigned long >	_firstSnapshot;		// contains threadIds of all threads present at the first snapshot
list< unsigned long >	_secondSnapshot;	// contains threadIds of all threads present at the second snapshot

bool snapshot( list< unsigned long >& );

// if passed in cpuId is set to -1, then bind calling thread to all cpus or otherwise clear all previous binds
//
// returns bound thread's threadId
bool bindThread( long& threadId, long& cpuId )
{
#ifdef WIN32

	DWORD_PTR cpuMask = 1ULL << cpuId;

	// learn all available cpus for this process
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;
	GetProcessAffinityMask( GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity );

	// assign all cpus if cpu asked for is not available
	if ( cpuId < 0 ||
		( cpuId > sizeof( DWORD_PTR ) * 8 ) ||
		!( cpuMask & dwprocessAffinity ) )
	{
		if (cpuId >= 0) printf("\nError: Requested CPU %d was not available, binding to all CPUs instead.\n\n", cpuId);
		cpuMask = dwprocessAffinity;
		cpuId = -1;
	}

	DWORD_PTR prevMask = 0;

	if ( threadId <= 0 )
	{
		threadId = (long) GetCurrentThreadId();

		// since handle was not assigned, bind the calling thread
		prevMask = SetThreadAffinityMask( GetCurrentThread(), cpuMask );
	}
	else
	{
		// since handle was assigned a valid value, bind the identified thread
		HANDLE hThread = OpenThread( THREAD_QUERY_INFORMATION | THREAD_SET_INFORMATION | THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME,
			FALSE, threadId );

		prevMask = SetThreadAffinityMask( hThread, cpuMask );

		CloseHandle( hThread );
	}

	if ( prevMask == 0 )
	{
		printf("\nError: Failed to bind thread with threadId %d to CPU %d. Last error is %d\n\n", threadId, cpuId, GetLastError());
		return false;
	}

	return true;

#else

	// learn all available cpus for this process
	cpu_set_t cpuSet;
	CPU_ZERO( &cpuSet );

	long cpuCount = sysconf( _SC_NPROCESSORS_ONLN );
	if ( cpuId < 0 || cpuId > cpuCount )
	{
		if ( cpuId >= 0 ) printf("\nError: Requested CPU %d was not available, binding to all CPUs instead.\n\n", cpuId);

		// this cpu is not available, assign all available cpus
		for ( int pos = 0; pos < 32; ++pos )
			if ( pos < cpuCount ) CPU_SET( pos, &cpuSet );

		cpuId = -1;
	}
	else
	{
		// this cpuId is available, just use it
		CPU_SET( (int) cpuId, &cpuSet );
	}

	// assign calling thread's thread id if not passed in yet
	if ( threadId <= 0 ) threadId = syscall( SYS_gettid );

	int ret = sched_setaffinity( threadId, sizeof( cpu_set_t ), &cpuSet );

	if ( ret != 0 )
	{
		printf("\nError: Failed to bind thread with threadId %d to CPU %d. Last error is %d\n\n", threadId, cpuId, errno);
		return false;
	}

	return true;

#endif
}

void bindThisThread( const EmaString& threadName, long cpuId, long threadId )
{
	_collectionLock.lock();

	long origCpuId = cpuId;

	bindThread( threadId, cpuId );

	ThreadInfo info;
	info.setBoundCPU( cpuId );
	info.setRequestedCPU( origCpuId );
	info.setThreadId( threadId );
	info.setThreadName( threadName );

	_collection.push_back( info );

	_collectionLock.unlock();
}

void firstThreadSnapshot()
{
	_collectionLock.lock();

	// take a snapshot of the system
	snapshot( _firstSnapshot );
}

void secondThreadSnapshot( const EmaString& threadName, long cpu )
{
	// wait a second so the system can start all the threads we look for
	AppUtil::sleep(1000);

	// take a snapshot of the system
	snapshot( _secondSnapshot );

	// check for new threads

	for ( list< unsigned long >::iterator iter = _firstSnapshot.begin();
		iter != _firstSnapshot.end(); ++iter )
		_secondSnapshot.remove( *iter );

	_collectionLock.unlock();

	// bind the one(s) that just showed up
	// 
	// note that the first thread is the actual EMA thread
	for ( list< unsigned long >::iterator iter = _secondSnapshot.begin();
		iter != _secondSnapshot.end(); ++iter )
		bindThisThread( threadName, cpu, *iter );
}

long getCpuCount()
{
#ifdef WIN32

	// learn all available cpus for this process
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;
	GetProcessAffinityMask( GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity );

	DWORD_PTR cpuMask = 1;
	long cpuCount = 0;
	while ( cpuMask )
	{
		cpuCount += ( cpuMask & dwprocessAffinity ) ? 1 : 0;
		cpuMask <<= 1;
	}

	return cpuCount;
#else
	return sysconf( _SC_NPROCESSORS_ONLN );
#endif
}

void printAllThreadBinding()
{
	_collectionLock.lock();

	EmaString temp( "\n\nThread Binding:" );
	temp.append( "\nThere are " );
	Int64 cpuCount = (Int64) getCpuCount();
	temp.append( cpuCount );
	temp.append( " CPUs (0 - " );
	temp.append( cpuCount - 1 );
	temp.append( ") available on this machine for thread binding." );

	temp.append( "\n\n----------------------------------------------------" );
	temp.append( "\nTID\tCPU #\tName\n----------------------------------------------------" );

	for ( list< ThreadInfo >::iterator iter = _collection.begin();
		iter != _collection.end(); ++iter )
	{
		temp.append( "\n" );
		temp.append( (Int64) (*iter).getThreadId() );
		temp.append( "\t" );

		if ( ( *iter ).getBoundCPU() < 0 )
			temp.append( "all" );
		else
			temp.append( (Int64)( *iter ).getBoundCPU() );

		temp.append( "\t" );
		temp += (*iter).getThreadName();

		if ( (*iter ).getBoundCPU() != ( *iter ).getRequestedCPU() )
		{
			temp.append( "\t" );
			temp.append( "requested CPU was " );

			if ( ( *iter ).getRequestedCPU() < 0 )
				temp.append( "all" );
			else
				temp.append( (Int64) ( *iter ).getRequestedCPU() );
		}
	}

	cout << temp << endl << endl;

	_collectionLock.unlock();
}

ThreadInfo& ThreadInfo::operator=( const ThreadInfo& other )
{
	if ( this == &other ) return *this;

	_threadId = other._threadId;
	_threadName = other._threadName;
	_requestedCPU = other._requestedCPU;
	_boundCPU = other._boundCPU;

	return *this;
}

ThreadInfo::ThreadInfo( const ThreadInfo& other )
{
	_threadId = other._threadId;
	_threadName = other._threadName;
	_requestedCPU = other._requestedCPU;
	_boundCPU = other._boundCPU;
}

bool ThreadInfo::operator==( const ThreadInfo& other ) const
{
	if ( this == &other ||
		( _threadId == other._threadId &&
		_threadName == other._threadName &&
		_requestedCPU == other._requestedCPU &&
		_boundCPU == other._boundCPU ) )
		return true;

	return false;
}

#ifdef WIN32

bool snapshot( list< unsigned long >& snapshotCollection )
{
	snapshotCollection.clear();

	THREADENTRY32 te32;

	// get a snapshot of the process at this time
	HANDLE hSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, GetCurrentProcessId() );
	if ( hSnap == INVALID_HANDLE_VALUE )
	{
		printf("CreateToolhelp32Snapshot() failed\n");
		return false;
	}

	memset( &te32, 0, sizeof( te32 ) );
	te32.dwSize = sizeof( te32 );
	if ( Thread32First( hSnap, &te32 ) == FALSE )
	{
		printf("Thread32First() failed\n");
		CloseHandle( hSnap );
		return false;
	}

	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId() );
	if ( hProcess == NULL )
	{
		printf("OpenProcess() failed\n");
		CloseHandle( hSnap );
		return false;
	}

	// walk the list of all threads currently in this process and save their ids
	do
	{
		if ( te32.th32OwnerProcessID != GetCurrentProcessId() )
			continue;

		HANDLE hThread = OpenThread( THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID );

		if ( !hThread ) continue;

		snapshotCollection.push_back( te32.th32ThreadID );

		CloseHandle( hThread );

	}
	while ( Thread32Next( hSnap, &te32 ) );

	CloseHandle( hProcess );

	CloseHandle( hSnap );

	return true;
}

#else

bool snapshot( list< unsigned long >& snapshotCollection )
{
	snapshotCollection.clear();

	// learn id of this process
	pid_t myPid = getpid();

	// open the system file with thread data
	EmaString directoryName( "/proc/" );
	directoryName.append( (UInt64) myPid );
	directoryName.append( "/task" );

	// open directory of this process's task
	DIR* dir = opendir( directoryName.c_str() );

	if ( !dir )
	{
		cout << "failed to open directory " << directoryName << endl;
		return false;
	}

	// collect all thread ids from this directory
	while ( 1 )
	{
		struct dirent* entry = readdir( dir );
		if ( !entry ) break;

		EmaString threadIdString( entry->d_name );

		// skip "." and ".."
		if ( threadIdString == EmaString( "." ) ||
			threadIdString == EmaString( ".." ) )
			continue;

		unsigned long threadId = atoi( entry->d_name );
		snapshotCollection.push_back( threadId );
	}

	// close the directory
	if ( closedir( dir ) )
	{
		cout << "failed to close directory " << directoryName << endl;
		return false;
	}

	return true;
}

#endif
