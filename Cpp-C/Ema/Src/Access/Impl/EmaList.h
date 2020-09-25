/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_EmaList_h
#define __refinitiv_ema_access_EmaList_h

#include "Common.h"
#include "Utilities.h"

namespace rtsdk {

namespace ema {

namespace access {

template< class T > struct EmaListElement;

template< class T >
class ListLinks
{
public:
	ListLinks() : _previous( 0 ), _next( 0 ) {}
	void setListPointers( T * previous, T * next )
	{
			_previous = previous;
			_next = next;
	}
	void setPreviousPointer( T * previous )
	{
		_previous = previous;
	}
	void setNextPointer( T * next )
	{
		_next = next;
	}
	T * next() const { return _next; }
	T * previous() const { return _previous; }
	void next(T * t) { _next = t; }
	void previous(T * t) { _previous = t; }
private:
	T * _next;
	T * _previous;
};

template< class T >
class EmaList
{
public:

	EmaList() {
		EMA_ASSERT( false, "EmaList only supported for pointer types" );
	}
};

template< class T >
class EmaList< T* >
{
public:

	EmaList();
	~EmaList();
	void clear();
	bool empty() const;
	UInt32 size() const;
	void push_back( T* );
	void push_front( T* );
	T* pop_back();
	T* pop_front();
	T * back() const;
	T * front() const;
	void insert( T* );
	void remove( T* );

private:
	UInt32 _size;
	T * _list;
	T * _last;
};

template< class T >
EmaList< T* >::EmaList() : _size( 0 ), _list( 0 ), _last(0) {}

template< class T >
EmaList< T* >::~EmaList() {}

template< class T >
void
EmaList< T* >::clear()
{
    _list = _last = 0;
    _size = 0;
}

template< class T >
bool
EmaList< T* >::empty() const
{
	return _size == 0;
}

template< class T >
UInt32 
EmaList< T* >::size() const
{
	return _size;
}

template< class T >
void
EmaList< T* >::push_back( T * element ) {
	if ( _last )
	{
		_last->setNextPointer( element );
        element->setPreviousPointer( _last );
		_last = element;
	}
	else
		_list = _last = element;

	++_size;
}

template< class T >
void
EmaList< T* >::push_front( T* element )
{
    if ( _list )
	{
        _list->setPreviousPointer( element );
        element->setNextPointer( _list );
        _list = element;
	}
	else
        _list = _last = element;

    ++_size;
}

template< class T >
T*
EmaList< T* >::pop_back()
{
	T* returnValue( _last );

	if ( _last )
	{
		T* nextBack( _last->previous() );
		_last->setListPointers( 0, 0 );

		if ( nextBack )
		{
			nextBack->next( 0 );
			_last = nextBack;
		}
		else
			_list = _last = 0;

		--_size;
	}

	return returnValue;
}

template< class T >
T *
EmaList< T* >::pop_front()
{
	T* returnValue( _list );

	if ( _list )
	{
		T * nextFront( _list->next() );
		_list->setListPointers( 0, 0 );

		if ( nextFront )
		{
			nextFront->setPreviousPointer( 0 );
			_list = nextFront;
		}
		else
			_list = _last = 0;

		--_size;

	}
	
	return returnValue;
}

template< class T >
T *
EmaList< T* >::front() const
{
    return _list;
}

template< class T >
T *
EmaList< T* >::back() const
{
    return _last;
}

template< class T >
void
EmaList< T* >::remove( T* element )
{
	if ( element->previous() )
		element->previous()->next( element->next() );
	else {
		_list = element->next();
		if ( _list )
			_list->previous( 0 );
	}

	if ( element->next() )
		element->next()->previous( element->previous() );
	else
	{
		_last = element->previous();
		if ( _last)
			_last->next( 0 );
	}
	element->setListPointers( 0, 0 );
	--_size;
}

template< class T >
void
EmaList< T* >::insert( T * item )
{
	if ( _size )
	{
		T * listItem = front();
		while ( listItem && *listItem < *item )
			listItem = listItem->next();
		if ( listItem )
		{
			if ( listItem->previous() )
			{
				listItem->previous()->setNextPointer( item );
				item->setPreviousPointer( listItem->previous() );
				listItem->setPreviousPointer( item );
				item->setNextPointer( listItem );
				++_size;
			}
			else
				push_front( item );
		}
		else
			push_back( item );
	}
	else
		push_back( item );
}

}

}

}

#endif

