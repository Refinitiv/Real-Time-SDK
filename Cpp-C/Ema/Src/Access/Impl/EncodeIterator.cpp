/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "EncodeIterator.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

#include <new>

using namespace thomsonreuters::ema::access;

EncodeIterator::EncodeIterator() :
 _rsslEncIter(),
 _rsslEncBuffer1(),
 _rsslEncBuffer2(),
 _allocatedSize( 0 ),
 _rsslMajVer( RSSL_RWF_MAJOR_VERSION ),
 _rsslMinVer( RSSL_RWF_MINOR_VERSION )
{
	rsslClearEncodeIterator( &_rsslEncIter );
}

EncodeIterator::~EncodeIterator()
{
	if ( _rsslEncBuffer1.data )
		delete [] _rsslEncBuffer1.data;

	if ( _rsslEncBuffer2.data )
		delete [] _rsslEncBuffer2.data;
}

void EncodeIterator::clear(UInt32 allocatedSize)
{
	if ( !_allocatedSize )
	{
		_allocatedSize = allocatedSize;

		try {
			_rsslEncBuffer1.data = new char[ _allocatedSize ];
		}
		catch ( std::bad_alloc )
		{
			rsslClearBuffer( &_rsslEncBuffer1 );

			_allocatedSize = 0;

			const char* temp = "Failed to allocate memory for encode iterator buffer in EncodeIterator::clear().";
			throwMeeException( temp );
			return;
		}

		*_rsslEncBuffer1.data = 0;
		_rsslEncBuffer1.length = _allocatedSize;

		rsslClearEncodeIterator( &_rsslEncIter );

		RsslRet retCode = rsslSetEncodeIteratorRWFVersion( &_rsslEncIter, _rsslMajVer, _rsslMinVer );
		if ( retCode != RSSL_RET_SUCCESS )
		{
			const char* temp = "Failed to set RsslEncodeIterator version in EncodeIterator::clear().";
			throwIueException( temp, retCode );
		}

		retCode = rsslSetEncodeIteratorBuffer( &_rsslEncIter, &_rsslEncBuffer1 );
		if ( retCode != RSSL_RET_SUCCESS )
		{
			const char* temp = "Failed to set RsslEncodeIterator buffer in EncodeIterator::clear().";
			throwIueException( temp, retCode );
		}
	}
	else if (_allocatedSize < allocatedSize)
	{
		reallocate( allocatedSize );
	}
	else
	{
		if ( _rsslEncBuffer1.data )
		{
			*_rsslEncBuffer1.data = 0;
			_rsslEncBuffer1.length = _allocatedSize;

			rsslClearEncodeIterator( &_rsslEncIter );

			RsslRet retCode = rsslSetEncodeIteratorRWFVersion( &_rsslEncIter, _rsslMajVer, _rsslMinVer );
			if ( retCode != RSSL_RET_SUCCESS )
			{
				const char* temp = "Failed to set RsslEncodeIterator version in EncodeIterator::clear().";
				throwIueException( temp, retCode );
			}

			retCode = rsslSetEncodeIteratorBuffer( &_rsslEncIter, &_rsslEncBuffer1 );
			if ( retCode != RSSL_RET_SUCCESS )
			{
				const char* temp = "Failed to set RsslEncodeIterator buffer in EncodeIterator::clear().";
				throwIueException( temp, retCode );
			}
		}
		else
		{
			*_rsslEncBuffer2.data = 0;
			_rsslEncBuffer2.length = _allocatedSize;

			rsslClearEncodeIterator( &_rsslEncIter );

			RsslRet retCode = rsslSetEncodeIteratorRWFVersion( &_rsslEncIter, _rsslMajVer, _rsslMinVer );
			if ( retCode != RSSL_RET_SUCCESS )
			{
				const char* temp = "Failed to set RsslEncodeIterator version in EncodeIterator::clear().";
				throwIueException( temp, retCode );
			}

			retCode = rsslSetEncodeIteratorBuffer( &_rsslEncIter, &_rsslEncBuffer2 );
			if ( retCode != RSSL_RET_SUCCESS )
			{
				const char* temp = "Failed to set RsslEncodeIterator buffer in EncodeIterator::clear().";
				throwIueException( temp, retCode );
			}
		}
	}
}

void EncodeIterator::reallocate()
{
	try {
		RsslUInt32 newSize = _allocatedSize << 1;

		RsslBuffer* temp1 = _rsslEncBuffer1.data ? &_rsslEncBuffer1 : &_rsslEncBuffer2;
		RsslBuffer* temp2 = _rsslEncBuffer1.data ? &_rsslEncBuffer2 : &_rsslEncBuffer1;

		temp2->data = new char[ newSize ];
		temp2->length = newSize;
		RsslRet retCode = rsslRealignEncodeIteratorBuffer( &_rsslEncIter, temp2 );

		if ( temp1->data )
		{
			delete [] temp1->data;
			rsslClearBuffer( temp1 );
		}

		if ( retCode != RSSL_RET_SUCCESS )
		{
			const char* temp = "Failed to realign RsslEncodeIterator buffer in EncodeIterator::reallocate().";
			throwIueException( temp, retCode );
		}

		_allocatedSize = newSize;
	}
	catch ( std::bad_alloc )
	{
		const char* temp = "Failed to allocate memory for encode iterator buffer in EncodeIterator::reallocate().";
		throwMeeException( temp );
	}
}

void EncodeIterator::reallocate( UInt32 size )
{
	if ( size < _allocatedSize ) return ;

	try {
		
		RsslBuffer* temp = _rsslEncBuffer1.data ? &_rsslEncBuffer1 : &_rsslEncBuffer2;

		char* tempBuffer = new char[ size ];

		memcpy( tempBuffer, temp->data, _allocatedSize );

		delete [] temp->data;

		temp->data = tempBuffer;
		temp->length = size;

		_allocatedSize = size;

		rsslClearEncodeIterator( &_rsslEncIter );

		RsslRet retCode = rsslSetEncodeIteratorRWFVersion( &_rsslEncIter, _rsslMajVer, _rsslMinVer );
		if ( retCode != RSSL_RET_SUCCESS )
		{
			const char* temp = "Failed to set RsslEncodeIterator version in EncodeIterator::reallocate().";
			throwIueException( temp, retCode );
		}

		retCode = rsslSetEncodeIteratorBuffer( &_rsslEncIter, temp );
		if ( retCode != RSSL_RET_SUCCESS )
		{
			const char* temp = "Failed to set RsslEncodeIterator buffer in EncodeIterator::reallocate().";
			throwIueException( temp, retCode );
		}
	}
	catch ( std::bad_alloc )
	{
		const char* temp = "Failed to allocate memory for encode iterator buffer in EncodeIterator::reallocate().";
		throwMeeException( temp );
	}
}

void EncodeIterator::setEncodedLength( UInt32 length )
{
	RsslBuffer* temp = _rsslEncBuffer1.data ? &_rsslEncBuffer1 : &_rsslEncBuffer2;

	temp->length = length;
}
