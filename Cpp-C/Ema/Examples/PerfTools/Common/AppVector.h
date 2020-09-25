///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef _EMA_COMMON_UTIL_EMA_VECTOR_
#define _EMA_COMMON_UTIL_EMA_VECTOR_

#include "Ema.h"
#include <assert.h>

namespace perftool {

namespace common {

/**
	\class AppVector
	\brief
	AppVector class provides template list implementation.

	Note: usage of AppVector requires template class T to provide
	operator==() and operator=().


	Example usage:

	\code

	class StringList : public AppVector< EmaString >
	{
		...
	};

	// or

	typedef AppVector< EmaString > StringList;

	// then

	StringList myList;

	// iterate through this list
	for ( UInt pos = 0; pos < myList.size(); ++pos )
	{
		if ( myList[pos] == EmaString( "abc" );
			break;
	}

	\endcode
*/
template< class T > class AppVector
{
public :

	///@name Constructor
	//@{
		/** default constructor
			\remark sets initial list capacity to zero
		*/
	AppVector( refinitiv::ema::access::UInt64 capacity = 0 );
	//@}

	///@name Copy Constructor
	//@{
		/** copy constructor
		*/
		AppVector( const AppVector< T >& other );
	//@}


	///@name Assignment Operator
	//@{
		/** assignment operator
		*/
		AppVector< T >& operator=( const AppVector< T >& other );
	//@}

	///@name Destructor
	//@{
		/** destructor
		*/
		virtual ~AppVector();
	//@}

	///@name Accessors
	//@{
		/** method to check if the list is empty
		*/
		bool empty() const;

		/** method to obtain current size of the list
			\return number of elements on the list
		*/
		refinitiv::ema::access::UInt64 size() const;

		/** index operator
			\remark allows read only access to the i-th element of the list
		*/
		const T& operator[]( refinitiv::ema::access::UInt64 index ) const;

		/** index operator
			\remark allows read & write access to the i-th element of the list
		*/
		T& operator[]( refinitiv::ema::access::UInt64 index );

		/** returns position of the first encountered element on the list
			that matches passed in value
			\param value - value of the element to be found
			\return -1 if no matching element was found and position otherwise
		*/
		refinitiv::ema::access::Int64 getPositionOf( const T& value ) const;

		/** comparison operator for the entire list
		*/
		bool operator==( const AppVector< T >& other ) const;
	//@}
		
	///@name Operations
	//@{
		/** clear the list
		*/
		void clear();

		/** method to push new entries on to the back of the list
			\remark will automatically resize if needed
		*/
		void push_back( const T& listElement );

		/** Removes position specified element from the list
			\param pos position of the element to be removed
			\return true if this element was removed, false otherwise
		*/
		bool removePosition( refinitiv::ema::access::UInt64 pos );

		/** Removes value specified element from the list
			\param value - value of the element to be removed
			\return true if this element was found and removed, false otherwise
			\remark will remove the very first encountered element (e.g., with lowest position)
			matching passed in value
		*/
		bool removeValue( const T& value );
	//@}

private :

	// data members
	
	refinitiv::ema::access::UInt64		m_capacity;

	refinitiv::ema::access::UInt64		m_size;

	T*			m_list;
};

// default constructor
template< class T >
AppVector< T >::AppVector( refinitiv::ema::access::UInt64 capacity ) :
	m_size( 0 ),
	m_capacity( capacity ),
	m_list( 0 )
{
	if ( !m_capacity ) return;
	
	m_list = new T[ (unsigned int)m_capacity ];
}

// copy constructor
template< class T >
AppVector< T >::AppVector( const AppVector< T >& other ) :
	m_capacity( other.m_capacity ),
	m_size( 0 ),
	m_list( 0 )
{
	if ( ! m_capacity ) return;

	m_size = other.m_size;

	m_list = new T[ (unsigned int)m_capacity ];

	for ( refinitiv::ema::access::UInt64 pos = 0; pos < m_size; ++pos )
	{
		m_list[pos] = other.m_list[pos];
	}
}

// assignment operator
template< class T >
AppVector< T >& AppVector< T >::operator=( const AppVector< T >& other )
{
	if ( this == &other ) return *this;

	if ( m_capacity >= other.m_size )
	{
		// enough memory already allocated
		m_size = other.m_size;

		for ( refinitiv::ema::access::UInt64 pos = 0; pos < m_size; ++pos )
		{
			m_list[pos] = other.m_list[pos];
		}
	}
	else
	{
		// not enough memory allocated
		if ( m_list ) delete [] m_list;

		m_capacity = other.m_capacity;
		m_size = other.m_size;

		m_list = new T[ (unsigned int)(m_capacity) ];

		for ( refinitiv::ema::access::UInt64 pos = 0; pos < m_size; ++pos )
		{
			m_list[pos] = other.m_list[pos];
		}
	}

	return *this;
}

// assignment operator
template< class T >
bool AppVector< T >::operator==( const AppVector< T >& other ) const
{
	if ( m_size != other.m_size ) return false;

	for ( refinitiv::ema::access::UInt64 pos = 0; pos < m_size; ++pos )
	{
		if ( ! ( m_list[pos] == other.m_list[pos] ) ) return false;
	}
		
	return true;
}

// destructor
template< class T >
AppVector< T >::~AppVector()
{
	m_capacity = 0;
	m_size = 0;
	if ( m_list ) delete [] m_list;
}

// clear method
template< class T >
void AppVector< T >::clear()
{
	m_size = 0;
}

template < class T >
void AppVector< T >::push_back( const T& element )
{
	if ( m_size < m_capacity )
	{
		m_list[m_size] = element;
		++m_size;
	}
	else
	{
		refinitiv::ema::access::UInt64 i = 0;
		if ( m_capacity == 0 )
		{
			m_capacity = 5;
		}
		else
		{
			m_capacity = 2 * m_capacity;
		}

		T* tempList;

		tempList = new T[ (unsigned int)(m_capacity)];

		for ( i = 0; i < m_size; i++ )
			tempList[i] = m_list[i];
		
		if ( m_list ) delete [] m_list;

		m_list = tempList;

		m_list[ m_size ] = element;
		++m_size;
	}
}

template <class T >
refinitiv::ema::access::UInt64 AppVector< T >::size() const
{
	return m_size;
}

template < class T >
T& AppVector< T >::operator[]( refinitiv::ema::access::UInt64 position ) 
{
	assert ( position < m_size);
	return m_list[position];
}

template < class T >
const T& AppVector< T >::operator[]( refinitiv::ema::access::UInt64 position ) const
{
	assert ( position < m_size);
	return m_list[position];
}

template < class T >
refinitiv::ema::access::Int64 AppVector< T >::getPositionOf( const T& value ) const
{
	refinitiv::ema::access::Int64 position = -1;

	// check which element matches the passed one
	for ( refinitiv::ema::access::UInt64 idx = 0; idx < m_size; ++idx )
	{
		if ( operator[]( idx ) == value )
		{
			position = (refinitiv::ema::access::Int64)(idx);
			break;
		}
	}

	return position;
}

template < class T >
bool AppVector< T >::removePosition( refinitiv::ema::access::UInt64 position )
{
	// return if index out of range
	if ( position >= m_size ) return false;

	// move down elements above passed idx by one
	for ( refinitiv::ema::access::UInt64 i = position + 1; i < m_size; ++i )
	{
		m_list[ i - 1 ] = m_list[ i ];
	}

	--m_size;
	
	return true;
}

template < class T >
bool AppVector< T >::removeValue( const T& value )
{
	bool copy = false;

	// check which element matches the passed one
	for ( refinitiv::ema::access::UInt64 idx = 0; idx < m_size; ++idx )
	{
		if ( !( operator[]( idx ) == value )  && !copy )
		{
			// do nothing since no match was found yet
			continue;
		}
		else if ( operator[]( idx ) == value )
		{
			// found the match
			copy = true;
		}
		else
		{
			// move down by one all the elements above the matching one
			m_list[ idx - 1 ] = m_list[ idx ];
		}
	}
	
	// adjust the current size
	if ( copy) --m_size;

	return copy;
}

template < class T >
bool AppVector< T >::empty() const
{
	return ( m_size ) ? false : true;
}

} // common

} // perftool

#endif // _EMA_COMMON_UTIL_EMA_VECTOR_
