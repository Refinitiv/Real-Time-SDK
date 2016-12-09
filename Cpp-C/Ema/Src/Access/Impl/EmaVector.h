/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_EmaVector_h
#define __thomsonreuters_ema_access_EmaVector_h

#include "EmaString.h"
#include "ExceptionTranslator.h"

#include <new>

namespace thomsonreuters {
	
namespace ema {

namespace access {

/**
\class EmaVector
\brief
EmaVector class provides template vector implementation.
*/

template< class T > class EmaVector
{
public :

	///@name Constructor
	//@{
	/** default constructor
	\remark sets initial vector capacity to zero
	*/
	EmaVector( UInt32 capacity = 0 );
	//@}

	///@name Copy Constructor
	//@{
	/** copy constructor
	*/
	EmaVector( const EmaVector< T >& other );
	//@}

	///@name Assignment Operator
	//@{
	/** assignment operator
	*/
	EmaVector< T >& operator=( const EmaVector< T >& other );
	//@}

	///@name Destructor
	//@{
	/** destructor
	*/
	virtual ~EmaVector();
	//@}

	///@name Accessors
	//@{
	/** method to check if the vector is empty
	*/
	bool empty() const;

	/** method to obtain current size of the vector
	\return number of elements on the vector
	*/
	UInt32 size() const;

	/** method to obtain current capacity of the vector
	\returns the size of the storage space currently allocated for the vector, expressed in terms of elements
	*/
	UInt32 capacity() const;

	/** index operator
	\remark allows read only access to the i-th element of the vector
	*/
	const T& operator[]( UInt32 index ) const;

	/** index operator
	\remark allows read & write access to the i-th element of the vector
	*/
	T& operator[]( UInt32 index );

	/** returns position of the first encountered element on the vector
	that matches passed in value
	\param value - value of the element to be found
	\return -1 if no matching element was found and position otherwise
	*/
	Int64 getPositionOf( const T& value ) const;

	/** comparison operator for the entire list
	*/
	bool operator==( const EmaVector< T >& other ) const;
	//@}

	///@name Operations
	//@{
	/** clear the vector
	*/
	void clear();

	/** method to push new entries on to the back of the vector
	\remark will automatically resize if needed
	*/
	void push_back( const T& entry );

	/** Removes position specified element from the vector
	\param pos position of the element to be removed
	\return true if this element was removed, false otherwise
	*/
	bool removePosition( UInt32 pos );

	/** Removes value specified element from the vector
	\param value - value of the element to be removed
	\return true if this element was found and removed, false otherwise
	\remark will remove the very first encountered element (e.g., with lowest position)
	matching passed in value
	*/
	bool removeValue( const T& value );
	//@}

private :

	UInt32		_capacity;
	UInt32		_size;
	T*			_list;
};

template< class T >
EmaVector< T >::EmaVector(UInt32 capacity) :
	_size( 0 ),
	_capacity( capacity ),
	_list( 0 )
{
	if ( !_capacity ) return;
	
	_list = new T[ (unsigned int)_capacity ];
}

template< class T >
EmaVector< T >::EmaVector(const EmaVector< T >& other) :
	_capacity( other._capacity ),
	_size( 0 ),
	_list( 0 )
{
	if ( ! _capacity ) return;

	_size = other._size;

	_list = new T[ (unsigned int)_capacity ];

	for ( UInt32 pos = 0; pos < _size; ++pos )
	{
		_list[pos] = other._list[pos];
	}
}

template< class T >
EmaVector< T >& EmaVector< T >::operator=(const EmaVector< T >& other)
{
	if ( this == &other ) return *this;

	if ( _capacity >= other._size )
	{
		_size = other._size;

		for ( UInt32 pos = 0; pos < _size; ++pos )
		{
			_list[pos] = other._list[pos];
		}
	}
	else
	{
		if ( _list ) delete [] _list;

		_capacity = other._capacity;
		_size = other._size;

		_list = new T[ (unsigned int)(_capacity) ];

		for ( UInt32 pos = 0; pos < _size; ++pos )
		{
			_list[pos] = other._list[pos];
		}
	}

	return *this;
}

template< class T >
bool EmaVector< T >::operator==(const EmaVector< T >& other) const
{
	if ( _size != other._size ) return false;

	for ( UInt32 pos = 0; pos < _size; ++pos )
	{
		if ( ! ( _list[pos] == other._list[pos] ) ) return false;
	}
		
	return true;
}

template< class T >
EmaVector< T >::~EmaVector()
{
	_capacity = 0;
	_size = 0;
	if ( _list ) delete [] _list;
}

template< class T >
void EmaVector< T >::clear()
{
	_size = 0;
}

template < class T >
void EmaVector< T >::push_back( const T& entry )
{
	if ( _size < _capacity )
	{
		_list[_size] = entry;
		++_size;
	}
	else
	{
		UInt32 i = 0;
		if ( _capacity == 0 )
		{
			_capacity = 5;
		}
		else
		{
			_capacity = 2 * _capacity;
		}

		T* tempList;

		tempList = new T[ (unsigned int)(_capacity)];

		for ( i = 0; i < _size; i++ )
			tempList[i] = _list[i];
		
		if ( _list ) delete [] _list;

		_list = tempList;

		_list[ _size ] = entry;
		++_size;
	}
}

template <class T >
UInt32 EmaVector< T >::size() const
{
	return _size;
}

template <class T >
UInt32 EmaVector< T >::capacity() const
{
	return _capacity;
}

template < class T >
T& EmaVector< T >::operator[]( UInt32 position ) 
{
	if (position >= _size)
	{
		throwOorException(EmaString("Passed in position is out of range."));
	}

	return _list[position];
}

template < class T >
const T& EmaVector< T >::operator[]( UInt32 position ) const
{
	if (position >= _size)
	{
		throwOorException(EmaString("Passed in position is out of range."));
	}

	return _list[position];
}

template < class T >
Int64 EmaVector< T >::getPositionOf( const T& value ) const
{
	Int64 position = -1;

	for ( UInt32 idx = 0; idx < _size; ++idx )
	{
		if ( operator[]( idx ) == value )
		{
			position = (Int64)(idx);
			break;
		}
	}

	return position;
}

template < class T >
bool EmaVector< T >::removePosition( UInt32 position )
{
	if ( position >= _size ) return false;

	for ( UInt32 i = position + 1; i < _size; ++i )
	{
		_list[ i - 1 ] = _list[ i ];
	}

	--_size;
	
	return true;
}

template< class T >
bool
EmaVector< T >::removeValue( const T& value )
{
	UInt32 i( 0 );
	while ( i < _size && operator[]( i ) != value )
		++i;
	if ( i == _size )
		return false;
	for ( ++i; i < _size; ++i )
		_list[ i - 1 ] = _list[ i ];
	--_size;
	return true;
}

template < class T >
bool EmaVector< T >::empty() const
{
	return ( _size ) ? false : true;
}

}

}

}

#endif // __thomsonreuters_ema_access_EmaVector_h
