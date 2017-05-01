/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_HashTable_h
#define __thomsonreuters_ema_access_HashTable_h

#include "EmaString.h"
#include <string.h>
#include <new>

namespace thomsonreuters {

namespace ema {

namespace access {

template<class T>
class Hasher
{
public:
	size_t operator()( const T& ) const;
};

template<>
size_t Hasher<int>::operator()( const int& val ) const;

template<>
size_t Hasher<EmaString>::operator()( const EmaString& val ) const;

template<class T>
class Equal_To
{
public:
	bool operator()( const T&, const T& ) const;
};

template<>
bool Equal_To<int>::operator()( const int& x, const int& y ) const;

template<>
bool Equal_To<EmaString>::operator()( const EmaString& x, const EmaString& y ) const;

template<class KeyType, class ValueType, class Hasher, class Equal_To>
class HashTable;

template<class KeyType, class ValueType, class Hasher = Hasher<KeyType>, class Equal_To = Equal_To<KeyType> >
class HashTable
{
public:

	HashTable( UInt32 size = 513, double loadFactor = 0.7 );

	~HashTable();

	bool insert( const KeyType&, const ValueType& );

	ValueType& operator[]( const KeyType& );

	int erase( const KeyType& );

	void clear();

	bool empty() const { return elementCount == 0 ? true : false; }

	size_t size() const { return elementCount; }

	ValueType* find( const KeyType& ) const;

	void rehash( UInt32 );

private:

	struct Node
	{
	public:

		Node( HashTable& parent, const KeyType& k, const ValueType& v, Node* previous = 0, Node* next = 0 ) :
			parent( parent ), key( k ), value( v ), previous( previous ), next( next )
		{
			++parent.elementCount;
		}

		~Node()
		{
			--parent.elementCount;
		}

		KeyType key;
		ValueType value;
		Node* previous;
		Node* next;
		HashTable& parent;

	private:

		Node& operator=( const Node& );
		Node( const Node& );
	};

	Node** theTable;
	UInt32 tableSize;
	Hasher hashFn;
	Equal_To keyEqual;
	UInt32 occupiedBucketCount;
	UInt32 elementCount;

	UInt32 rehashWhen;
	double loadFactor;

	HashTable( const HashTable& );
	HashTable& operator=( const HashTable& );

	void _insert( Node*& location, Node* previous, const KeyType& key, const ValueType& value )
	{
		location = new Node( *this, key, value, previous, 0 );
		return;
	}
};

template<class KeyType, class ValueType, class Hasher, class Equal_To>
HashTable<KeyType, ValueType, Hasher,  Equal_To>::HashTable( UInt32 size, double loadFactor ) :
	tableSize( size == 0 ? 513 : size ), elementCount( 0 ), occupiedBucketCount( 0 ), loadFactor( loadFactor )
{
	theTable = new Node *[tableSize];
	memset( theTable, 0, tableSize * sizeof theTable[0] );
	rehashWhen = static_cast<UInt32>( tableSize * loadFactor );
}

template<class KeyType, class ValueType, class Hasher, class Equal_To>
HashTable<KeyType, ValueType, Hasher,  Equal_To>::~HashTable()
{
	Node* next( 0 );
	for ( UInt32 i = 0; i < tableSize; ++i )
		for ( Node* p = theTable[i]; p; p = next )
		{
			next = p->next;
			delete p;
		}
	delete [] theTable;
}

template<class KeyType, class ValueType, class Hasher, class Equal_To>
bool
HashTable<KeyType, ValueType, Hasher, Equal_To>::insert( const KeyType& key, const ValueType& value )
{
	size_t slot = hashFn( key ) % tableSize;
	Node* bucket = *( theTable + slot );

	if ( bucket )
	{
		Node* p = bucket;
		while ( true )
		{
			if ( keyEqual( p->key, key ) )
				return false;

			if ( p->next )
				p = p->next;
			else
				break;
		}

		_insert( p->next, p, key, value );
	}

	else
	{
		_insert( *( theTable + slot ), 0, key, value );
		if ( ++occupiedBucketCount >= rehashWhen )
		{
			rehash( tableSize * 2 + 1 );
			return true;
		}
	}

	return true;
}

template<class KeyType, class ValueType, class Hasher, class Equal_To>
ValueType&
HashTable<KeyType, ValueType, Hasher, Equal_To>::operator[]( const KeyType& key )
{
	size_t slot = hashFn( key ) % tableSize;
	Node* bucket = *( theTable + slot );
	UInt32 bucketSize( 0 );

	if ( bucket )
	{
		Node* p;
		Node* last( 0 );
		for ( p = bucket; p; last = p, p = p->next )
			if ( keyEqual( p->key, key ) )
				return p->value;

		ValueType v;
		_insert( last->next, last, key, v );
	
		return last->next->value;
	}

	else
	{
		static ValueType v;
		_insert( *( theTable + slot ), 0, key, v );
		++occupiedBucketCount;

		if ( ++occupiedBucketCount >= rehashWhen )
		{
			rehash( tableSize * 2 + 1 );
			slot = hashFn( key ) % ( tableSize );
			Node* p = theTable[slot];
			while ( p )
			{
				if ( keyEqual( p->key, key ) )
					return p->value;
				p = p->next;
			}
		}

		return theTable[slot]->value;
	}
}

template<class KeyType, class ValueType, class Hasher, class Equal_To>
int
HashTable<KeyType, ValueType, Hasher, Equal_To>::erase( const KeyType& key )
{
	size_t slot = hashFn( key ) % tableSize;
	for ( Node* p = *( theTable + slot ); p; p = p->next )
	{
		if ( keyEqual( p->key, key ) )
		{
			if ( p == *( theTable + slot ) )
			{
				*( theTable + slot ) = p->next;
				if ( p->next )
					p->next->previous = 0;
				else
					--occupiedBucketCount;
			}
			else
			{
				p->previous->next = p->next;
				if ( p->next )
					p->next->previous = p->previous;
			}

			delete p;
			p = 0;
			return 1;
		}
	}

	return 0;
}

template<class KeyType, class ValueType, class Hasher, class Equal_To>
void
HashTable<KeyType, ValueType, Hasher, Equal_To>::clear()
{
	Node* next( 0 );
	for ( UInt32 i = 0; i < tableSize; ++i )
	{
		for ( Node* p = theTable[i]; p; p = next )
		{
			next = p->next;
			delete p;
		}
		theTable[i] = 0;
	}
	occupiedBucketCount = elementCount = 0;
}

template<class KeyType, class ValueType, class Hasher, class Equal_To>
ValueType*
HashTable<KeyType, ValueType, Hasher, Equal_To>::find( const KeyType& key ) const
{
	size_t slot = hashFn( key ) % tableSize;
	for ( Node* p = *( theTable + slot ); p != 0; p = p->next )
		if ( keyEqual( p->key, key ) )
			return &( p->value );
	return 0;
}

template<class KeyType, class ValueType, class Hasher, class Equal_To>
void
HashTable<KeyType, ValueType, Hasher, Equal_To>::rehash( UInt32 newSize )
{
	if ( newSize <= tableSize )
		return;

	Node** oldHashTable = theTable;
	UInt32 oldHashTableSize( tableSize );

	tableSize = newSize;
	theTable = new Node *[tableSize];
	memset( theTable, 0, tableSize * sizeof theTable[0] );
	occupiedBucketCount = 0;

	Node* element;
	Node* next;
	for ( UInt32 i = 0; i < oldHashTableSize; ++i )
	{
		element = oldHashTable[i];
		while ( element )
		{
			next = element->next;
			size_t slot = hashFn( element->key ) % tableSize;
			if ( theTable[slot] )
			{
				theTable[slot]->previous = element;
				element->next = theTable[slot];
			}
			else
				element->next = 0;

			element->previous = 0;
			theTable[slot] = element;

			element = next;
		}
	}

	for ( size_t i = 0; i < tableSize; ++i )
		if ( theTable[i] )
		{
			++occupiedBucketCount;
		}
	rehashWhen = static_cast<UInt32>( tableSize * loadFactor );

	delete [] oldHashTable;
}

}

}

}

#endif // __thomsonreuters_ema_access_HashTable_h
