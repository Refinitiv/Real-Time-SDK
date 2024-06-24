///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#ifndef _EMA_COMMON_UTIL_CTRL_BREAK_HANDLER_H_
#define _EMA_COMMON_UTIL_CTRL_BREAK_HANDLER_H_

#include "Ema.h"

#if defined(WIN32)
	#include <windows.h>
#else
	#include <unistd.h>
	#include <signal.h>
	#include <strings.h>
#endif

namespace perftool {

namespace common {

/**
	\class CtrlBreakHandler
	\brief
	CtrlBreakHandler class provides handling of CTRL-C and CTRL-BREAK signals.

	Example usage:

	\code

	while ( ! CtrlBreakHandler::isTerminated() )
	{
		...
	}

	\endcode
*/
class CtrlBreakHandler
{
public:

	///@name Constructor
	//@{
		/** default constructor
		*/
		CtrlBreakHandler();
	//@}

	///@name Constructor
	//@{
		/** destructor
		*/
		virtual ~CtrlBreakHandler();
	//@}

	///@name Operations
	//@{
		/** method to force application exit
		*/
		static void forceExit();
	//@}

	///@name Accessors
	//@{
		/** method to learn if CTRL-C or CTRL_BREAK was pressed
		*/
		static bool isTerminated() { return m_isTerminated; }
	//@}

	///@name Register action handler
	//@{
		/** method to register a handler for CTRL-C or CTRL_BREAK
		*/
		static void registerAction();
	//@}

private:

	static bool m_isTerminated;

	// handler routine specification
#if defined(WIN32)
	static BOOL WINAPI TermHandlerRoutine( DWORD dwCtrlType );
#else
	static void sigAction( int sig, siginfo_t * pSiginfo, void* pv );

	static struct sigaction _sigAction;
	static struct sigaction _oldSigAction;
#endif 
};

} // common

} // perftool

#endif // _EMA_COMMON_UTIL_CTRL_BREAK_HANDLER_H_
