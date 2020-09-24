/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_Pool_h
#define __rtsdk_ema_access_Pool_h

#include "Mutex.h"
#include "EmaVector.h"
#include "ExceptionTranslator.h"

#include <new>

namespace rtsdk {

namespace ema {

namespace access {

template< class O >
class Factory
{
public :

	static O* create();

	static void destroy( O* o ) { delete o; }
};

template< class O >
O* Factory< O >::create()
{
	try {
		return new O;
	}
	catch ( std::bad_alloc& )
	{
		const char* temp = "Failed to create object in Factory< O >::create(). Out of memory.";
		throwMeeException( temp );
	}
	return 0;
}

template < class I >
class EncoderPool 
{
public :

	EncoderPool( UInt32 size );

	virtual ~EncoderPool();

	void clear();

	I* getItem();

	void returnItem( I* );

	UInt32 count();

private :

	Mutex				_lock;

	EmaVector< I* >		_vector;

	UInt32				_count;

	EncoderPool();
	EncoderPool( const EncoderPool& );
	EncoderPool& operator=( const EncoderPool& );
};

template< class I >
EncoderPool< I >::EncoderPool( UInt32 size ) :
 _count( 0 )
{
	for ( UInt32 idx = 0; idx < size; ++idx )
		_vector.push_back( 0 );
}

template< class I >
EncoderPool< I >::~EncoderPool()
{
	clear();
}

template< class I >
void EncoderPool< I >::clear()
{
	_lock.lock();

	if ( !_count )
	{
		_lock.unlock();
		return;
	}

	for ( UInt32 idx = _vector.size(); idx != 0; --idx )
	{
		I* temp = _vector[ idx - 1 ];
		if ( temp )
		{
			Factory< I >::destroy( temp );
			_vector[ idx - 1 ] = 0;
		}
	}

	_count = 0;

	_lock.unlock();
}

template< class I >
I* EncoderPool< I >::getItem()
{
	_lock.lock();

	if ( !_count )
	{
		_lock.unlock();

		return Factory< I >::create();
	}

	I*& itemRef = _vector[ --_count ];

	I* item = itemRef;

	itemRef = 0;
	
	_lock.unlock();

	return item;
}

template< class I >
void EncoderPool< I >::returnItem( I* item )
{
	item->clear();

	_lock.lock();

	if ( _count == _vector.capacity() )
		do { _vector.push_back( 0 ); } while ( _vector.size() < _vector.capacity() );

	_vector[ _count++ ] = item;

	_lock.unlock();
}

template< class I >
UInt32 EncoderPool< I >::count()
{
	return _count;
}

template < class I >
class DecoderPool 
{
public :

	DecoderPool( UInt32 size );

	virtual ~DecoderPool();

	void clear();

	I* getItem();

	void returnItem( I* );

	UInt32 count();

private :

	Mutex				_lock;

	EmaVector< I* >		_vector;

	UInt32				_count;

	DecoderPool();
	DecoderPool( const DecoderPool& );
	DecoderPool& operator=( const DecoderPool& );
};

template< class I >
DecoderPool< I >::DecoderPool( UInt32 size ) :
 _count( 0 )
{
	for ( UInt32 idx = 0; idx < size; ++idx )
		_vector.push_back( 0 );
}

template< class I >
DecoderPool< I >::~DecoderPool()
{
	clear();
}

template< class I >
void DecoderPool< I >::clear()
{
	_lock.lock();

	if ( !_count )
	{
		_lock.unlock();
		return;
	}

	for ( UInt32 idx = _vector.size(); idx != 0; --idx )
	{
		I* temp = _vector[ idx - 1 ];
		if ( temp )
		{
			temp->setAtExit();
			Factory< I >::destroy( temp );
			_vector[ idx - 1 ] = 0;
		}
	}

	_count = 0;

	_lock.unlock();
}

template< class I >
I* DecoderPool< I >::getItem()
{
	_lock.lock();

	if ( !_count )
	{
		_lock.unlock();

		return Factory< I >::create();
	}

	I*& itemRef = _vector[ --_count ];

	I* item = itemRef;

	itemRef = 0;
	
	_lock.unlock();

	return item;
}

template< class I >
void DecoderPool< I >::returnItem( I* item )
{
	_lock.lock();

	if ( _count == _vector.capacity() )
		do { _vector.push_back( 0 ); } while ( _vector.size() < _vector.capacity() );

	_vector[ _count++ ] = item;

	_lock.unlock();
}

template< class I >
UInt32 DecoderPool< I >::count()
{
	return _count;
}


template < class I, class T = I >
class Pool
{
public :

	Pool( UInt32 size );

	virtual ~Pool();

	void clear();

	I* getItem();

	void returnItem( I* );

	UInt32 count();

private :

	Mutex	_lock;

	EmaVector< I* >		_vector;

	UInt32				_count;

	Pool();
	Pool( const Pool& ); 
	Pool& operator=( const Pool& );
};

template< class I, class T >
Pool< I, T >::Pool( UInt32 size ) :
 _count( 0 )
{
	for ( UInt32 idx = 0; idx < size; ++idx )
		_vector.push_back( 0 );
}

template< class I, class T >
Pool< I, T >::~Pool()
{
	clear();
}

template< class I, class T >
void Pool< I, T >::clear()
{
	_lock.lock();

	if ( !_count )
	{
		_lock.unlock();
		return;
	}

	for ( UInt32 idx = _vector.size(); idx != 0; --idx )
	{
		I* temp = _vector[ idx - 1 ];
		if ( temp )
			Factory< I >::destroy( temp );
		_vector[ idx - 1 ] = 0;
	}

	_count = 0;

	_lock.unlock();
}

template< class I, class T >
I* Pool< I, T >::getItem()
{
	_lock.lock();

	if ( !_count )
	{
		_lock.unlock();

		return Factory< T >::create();
	}

	I*& itemRef = _vector[ --_count ];

	I* item = itemRef;

	itemRef = 0;
	
	_lock.unlock();

	return item;
}

template< class I, class T >
void Pool< I, T >::returnItem( I* item )
{
	_lock.lock();

	if ( _count == _vector.capacity() )
		do { _vector.push_back( 0 ); } while ( _vector.size() < _vector.capacity() ); 

	_vector[ _count++ ] = item;

	_lock.unlock();
}

template< class I, class T >
UInt32 Pool< I, T >::count()
{
	return _count;
}

}

}

}

#endif // __rtsdk_ema_access_Pool_h
