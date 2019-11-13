///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "ConsumerManager.h"

using namespace thomsonreuters::ema::access;

ConsumerManager::ConsumerManager( const EmaString& host, const EmaString& username ) :
 running( false )
{
	ommConsumer = new OmmConsumer( OmmConsumerConfig().host( host ).username( username ).operationModel( OmmConsumerConfig::UserDispatchEnum ) );
}

ConsumerManager::~ConsumerManager()
{
	stop();

	if ( ommConsumer )
	{
		delete ommConsumer;
		ommConsumer = 0;
	}
}

OmmConsumer & ConsumerManager::getOmmConsumer()
{
	return *ommConsumer;
}

void ConsumerManager::run()
{
	while ( running )
	{
		ommConsumer->dispatch( 1000 ); 
	}
}

#ifdef WIN32
unsigned __stdcall ConsumerManager::ThreadFunc( void* pArguments )
#else
extern "C" void * ConsumerManager::ThreadFunc( void* pArguments )
#endif
{
	((ConsumerManager *)pArguments)->run();

	return 0;
}

void  ConsumerManager::start()
{
#ifdef WIN32
	_handle = (HANDLE)_beginthreadex( NULL, 0, ThreadFunc, this, 0, &_threadId );
	assert( _handle != 0 );

	SetThreadPriority( _handle, THREAD_PRIORITY_NORMAL );
#else
	pthread_create( &_threadId, NULL, ThreadFunc, this );
	assert( _threadId != 0 );
#endif

	running = true;
}

void  ConsumerManager::stop()
{
	if ( running )
	{
		running = false;
#ifdef WIN32
		WaitForSingleObject( _handle, INFINITE );
		CloseHandle( _handle );
		_handle = 0;
		_threadId = 0;
#else
		pthread_join( _threadId, NULL );
		_threadId = 0;
#endif
	}

	running = false;
}
