///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "CtrlBreakHandler.h"

using namespace perftool::common;

CtrlBreakHandler m_ctrlBreakHandler;	// the singleton break handler object


// class CtrlBreakHandler implementation

bool CtrlBreakHandler::m_isTerminated = false;

#ifndef WIN32
struct sigaction CtrlBreakHandler::_sigAction;
struct sigaction CtrlBreakHandler::_oldSigAction;
#endif

// constructor
CtrlBreakHandler::CtrlBreakHandler()
{
#if defined(WIN32)
#else
	bzero( &_sigAction, sizeof( _sigAction ) );
	bzero( &_oldSigAction, sizeof( _oldSigAction ) );
#endif // WIN32
}

#if defined(WIN32)
void CtrlBreakHandler::registerAction()
{
	SetConsoleCtrlHandler(&CtrlBreakHandler::TermHandlerRoutine, TRUE);
}
#else
void CtrlBreakHandler::registerAction()
{
	struct sigaction tempSigAction;
	bzero( &tempSigAction, sizeof( tempSigAction ) );

	sigaction(SIGINT, NULL, &tempSigAction);
	if (tempSigAction.sa_sigaction != sigAction)
	{
		bzero( &_sigAction, sizeof( _sigAction ) );
		bzero( &_oldSigAction, sizeof( _oldSigAction ) );

		_sigAction.sa_sigaction = sigAction;
		_sigAction.sa_flags = SA_SIGINFO;

		sigaction( SIGINT, &_sigAction, &_oldSigAction );
	}
}
#endif

// destructor
CtrlBreakHandler::~CtrlBreakHandler()
{
#if defined(WIN32)
	SetConsoleCtrlHandler( &CtrlBreakHandler::TermHandlerRoutine, FALSE );
#else
	if (_oldSigAction.sa_sigaction != NULL)
		sigaction( SIGINT, &_oldSigAction, NULL );
#endif
}

#if defined(WIN32)
BOOL WINAPI CtrlBreakHandler::TermHandlerRoutine( DWORD dwCtrlType )
{
	switch ( dwCtrlType )
	{
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_C_EVENT:
		m_isTerminated = true;
		break;
	}
	return FALSE;
}
#else
extern "C" {
	void CtrlBreakHandler::sigAction( int sig, siginfo_t* pSiginfo, void* pv )
	{
		m_isTerminated = true;
		if (_oldSigAction.sa_sigaction != NULL)
			(_oldSigAction.sa_sigaction)(sig, pSiginfo, pv);
	}
}
#endif

void CtrlBreakHandler::forceExit()
{
#if defined(WIN32)
	GenerateConsoleCtrlEvent( CTRL_C_EVENT, 0 );
#else
	kill( getpid(), SIGINT );
#endif
}
