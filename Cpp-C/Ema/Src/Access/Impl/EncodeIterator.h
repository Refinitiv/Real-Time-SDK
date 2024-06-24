/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_EncodeIterator_h
#define __refinitiv_ema_access_EncodeIterator_h

#include "EmaPool.h"
#include "rtr/rsslIterators.h"

namespace refinitiv {

namespace ema {

namespace access {

class EncodeIterator
{
public :

	EncodeIterator();

	virtual ~EncodeIterator();

	void clear( UInt32 allocatedSize = 4096 );

	void reallocate();

	void reallocate( UInt32 );

	void setEncodedLength( UInt32 );

	RsslEncodeIterator		_rsslEncIter;

	RsslBuffer				_rsslEncBuffer1;

	RsslBuffer				_rsslEncBuffer2;

	RsslUInt32				_allocatedSize;

	RsslUInt8				_rsslMajVer;

	RsslUInt8				_rsslMinVer;
};

class EncodeIteratorPool : public Pool< EncodeIterator >
{
public :

	EncodeIteratorPool( unsigned int size = 5 ) : Pool< EncodeIterator >( size ) {};

	virtual ~EncodeIteratorPool() {}

private :

	EncodeIteratorPool( const EncodeIteratorPool& );
	EncodeIteratorPool& operator=( const EncodeIteratorPool& );
};

}

}

}

#endif //__refinitiv_ema_access_EncodeIterator_h
