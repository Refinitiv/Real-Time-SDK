/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) Refinitiv 2019-2020. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


#ifndef __RJC_SMALL_STRING
#define __RJC_SMALL_STRING

#ifndef _RDEV_NO_STL_
#include <iostream>
#else
#include <stdio.h>
#endif
#include <memory.h>

#include "rtr/rtrdefs.h"
#include "rtr/rtdataty.h"
#include "rtr/rjcstring.h"

// Synopsis:
// #include"rtr/rjcsmallstr.h"
//
// Description:
// See Also:
//	RJCString, RTRExternalValue, RTRListOfExternalValue
//

class RJCSmallString
{
#ifndef _RDEV_NO_STL_
friend std::ostream& operator<<(std::ostream& os, RJCSmallString& aSmallString);
#else
public:
void print(FILE*);
#endif
public:

// Constructors
	RJCSmallString();
		// Create a default empty small buffer
		// _TAG Constructor

	RJCSmallString(int len);
		// Create a small buffer of capacity `len'
		// REQUIRE : len < 256
        // _TAG Constructor

	RJCSmallString(	const char* n,
					int num_bytes,
					int cap);
		// Create a small buffer initialized with `num_bytes' bytes from `n' and
		// capacity of `cap'
		// REQUIRE : cap < 256
        // _TAG Constructor

	RJCSmallString(const RJCSmallString &aSmallString);
		// Copy constructor
        // _TAG Constructor

	RJCSmallString(const char* n);
		// Create a small buffer initialized with a null terminated string
        // _TAG Constructor

// Destructor
	~RJCSmallString();
		// _TAG Destructor

// Attributes
	inline int length() const;
		// The length of the Item Name.
		// _TAG01 Attributes

	inline int capacity() const;
		// The maximum capacity of the Item Name
        // _TAG01 Attributes

// Equality comparisons
	RTRBOOL operator==(const RJCSmallString& aSmallString);
		// _TAG02 Equality comparisons

	RTRBOOL operator!=(const RJCSmallString& aSmallString);
        // _TAG02 Equality comparisons

// Assignment
	RJCSmallString&	operator=(const RJCSmallString& aSmallString);
		// _TAG03 Assignment

// Setting
	inline void set( const char *cstr, int p1, int p2 );
		// Set Current to values of `cstr' starting at position
		// `p1' and ending at position `p2'. `p1' starts at 1.
		// The data to be placed into Current must fit its
		// current capacity().
		// _TAG04 Setting

	inline void set( const char *cstr, int len );
		// Set Current to values of `cstr' for `len' values.
		// The data to be placed into Current must fit its
		// current capacity().
        // _TAG04 Setting

// Setting and forcing
	inline void force( const char *cstr, int p1, int p2 );
		// Set Current to values of `cstr' starting at position
		// `p1' and ending at position `p2'. `p1' starts at 1.
		// Current will grow to fit if it needs to.
		// _TAG05 Setting and forcing

	inline void force( const char *cstr, int len );
		// Set Current to values of `cstr' for `len' values.
		// The data to be placed into Current must fit its
		// Current will grow to fit if it needs to.
        // _TAG05 Setting and forcing

// Resizing
	void resize( int new_size );
		// Make Current have capacity of at least `new_size' while
		// copying the current data over.
		// _TAG06 Resizing

	void grow( int new_size );
		// Make Current have capacity of at least `new_size'.
        // _TAG06 Resizing

// Conversion to standard C style char *
	inline operator const char*() const;
		// _TAG07 Conversion to standard C style char *

	inline const char *to_c() const;
        // _TAG07 Conversion to standard C style char *

// Conversion to RTString type.
	RJCString &getString();
		// _TAG08 Conversion to RTString type

// Hash values

private:
	static unsigned long Hash(const char * const data, const int);
		// A hashing code for the string, inline definition
		// Used to implement hash() and hashKeySmallString()

public:
	unsigned long hash() const;
		// A hashing code for the string
		// _TAG09 Hash values 

	static unsigned long hashKeySmallString( RJCSmallString *name );
		// A hashing code for the string. Used by hash tables.
        // _TAG09 Hash values
	static unsigned long hashBuffer(const char * const data, const int length);
protected:

	u_8		_length;
	u_8		_capacity;
	char *	_data;


// Constructors
	RJCSmallString(int num_bytes, const char * n, int cap);

};

inline int RJCSmallString::length() const {return( _length );};
inline int RJCSmallString::capacity() const {return( _capacity );};

inline const char *RJCSmallString::to_c() const
{
	// Assumed that _data is 1 greater
	// than _capacity.
	_data[_length] = '\0';
	return( _data );
};

inline RJCSmallString::operator const char*() const {return to_c();};

inline void RJCSmallString::set( const char *cstr, int p1, int p2 )
{
	RTPRECONDITION( p1 >= 1 );
	RTPRECONDITION( p2 >= p1 );
	RTPRECONDITION( (p2-p1+1) <= capacity() );

	_length = p2-p1+1;
	(void)memcpy( _data, ( cstr+p1-1 ), _length );
};

inline void RJCSmallString::set( const char *cstr, int len )
{
	RTPRECONDITION( len <= capacity() );
	_length = len;
	(void)memcpy( _data, cstr, (unsigned int)len );
};

inline void RJCSmallString::force( const char *cstr, int p1, int p2 )
{
	RTPRECONDITION( p1 >= 1 );
	RTPRECONDITION( p2 >= p1 );
	_length = p2-p1+1;
	grow( _length );
	(void)memcpy( _data, ( cstr+p1-1 ), _length );
};

inline void RJCSmallString::force( const char *cstr, int len )
{
	_length = len;
	grow( _length );
	(void)memcpy( _data, cstr, _length );
};

#endif
