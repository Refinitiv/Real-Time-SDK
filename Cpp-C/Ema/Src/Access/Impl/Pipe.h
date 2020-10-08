/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_Pipe_h
#define __refinitiv_ema_access_Pipe_h

namespace refinitiv {

namespace ema {

namespace access {

#define WinPipePort_Name		"WinPipePort"
#define WinPipePort_Default		9001

class Pipe
{
public:

	Pipe();
	virtual ~Pipe();

	bool create( int winPipePort = WinPipePort_Default );
	void close();

	int	read( void* buffer, int length );
	int	write( const void* buffer, int length );

	int readFD() const { return _fds[0]; }
	int	writeFD() const { return _fds[1]; }

	bool isInitialized() const { return _initialized; }

private : 

	int			_fds[2];
	
	bool		_initialized;

#ifdef WIN32
	bool socketpair( int& appFD, int& threadFD, int& port, int startport, int maxport );
#endif
	
	void closeFD( int idx );

	Pipe( const Pipe & );
	Pipe& operator= ( const Pipe & );
};

}

}

}

#endif // __refinitiv_ema_access_Pipe_h
