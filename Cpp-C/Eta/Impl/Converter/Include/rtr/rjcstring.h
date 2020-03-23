/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */


#ifndef _rjctring_h
#define _rjctring_h

#ifndef _RDEV_NO_STL_
#include <iostream>
#else
#include <stdio.h>
#endif

#include "rtr/rtrdefs.h"
#include "rtr/os.h"

#undef index	// may be #defined as strchr

// Synopsis:
// #include"rtr/rjcstring.h"
//
// Description:
//	A representation for a sequence of characters. The sequence may contain
//	embedded null characters.
//
// See Also:
//	RTRExternalValue, RTRListOfExternalValue
//

class RJCString
{

#ifndef _RDEV_NO_STL_
friend std::ostream& operator<<(std::ostream&, const RJCString&); 
#else
public:
void print(FILE*);
#endif
public:
// Constructors
	RJCString();
		// An empty string.
		// _TAG Constructor

	RJCString(unsigned int n);
		// A string with capacity n.
        // _TAG Constructor

	RJCString(char c, unsigned int n);
		// A string with capacity n, initialized with character c.
        // _TAG Constructor

	RJCString(const char *str);
		// A string with a copy of the null terminated string str.
        // _TAG Constructor

	RJCString(const char *str, unsigned int n);
		// A string with a copy of the n bytes from null terminated string str.
        // _TAG Constructor

	RJCString(const RJCString&);
		// A string with a copy of the n bytes from null terminated string str.
        // _TAG Constructor

// Destructor
	~RJCString();
		// _TAG Destructor

// Static attributes
	static int defaultCapacity;
		// The size of area allocated when using the default constructor.
	
	static char *trueString;
		// The value to which strings are compared when converting to a boolean.

// Attributes
	inline const unsigned int capacity() const;
		// The current capacity of this string.
		// _TAG01 Attributes

	inline unsigned int count() const;
		// The number of characters in this string.
        // _TAG01 Attributes

	inline RTRBOOL isEmpty() const;
		// Is this string empty. (result == RTRTRUE imples count() == 0)
        // _TAG01 Attributes

	unsigned long hash() const;
		// A hash value for this string.
        // _TAG01 Attributes

	inline int lower() const;
		// Minimum valid index for accessing this string.
        // _TAG01 Attributes

	inline int upper() const;
		// Maximmum valid index for accessing this string.
        // _TAG01 Attributes

// Modify - in entirety
	RJCString& set(const char *str, unsigned int p1, unsigned int p2);
		// Initialize this string to the contents of str, starting a position
		// p1 (0 based) and ending at position p2
		// REQUIRE : p1 <= p2
		// _TAG02 Modify - in entirety

	RJCString& set(const char *str, unsigned int n);
		// Initialize this string to the first n bytes of str.
        // _TAG02 Modify - in entirety

#ifndef _RDEV_NO_STL_
	RJCString& readLine(std::istream&, RTRBOOL skipWhite=1);
		// Set this string to a line extracted from the given stream.
        // _TAG02 Modify - in entirety
#endif

	inline RJCString& clear();
		// Empty this string.
		// ENSURE : isEmpty()
        // _TAG02 Modify - in entirety

	RJCString& fromNumeric(int i);
        // _TAG02 Modify - in entirety

	RJCString& fromNumeric(unsigned int i);
        // _TAG02 Modify - in entirety

	RJCString& fromNumeric(long i);
        // _TAG02 Modify - in entirety

	RJCString& fromNumeric(RTR_I64 i);
   		// _TAG02 Modify - in entirety

	RJCString& fromNumeric(unsigned long i);
        // _TAG02 Modify - in entirety

	RJCString& fromNumeric(double i);
		// Set this string to the ASCII representation of i.
        // _TAG02 Modify - in entirety

// Modify - in part
	char& operator[](int i);
		// Set the i'th character in this string.
		// REQUIRE: i >= lower()
		// REQUIRE: i <= upper()
		// _TAG03 Modify - in part

	RJCString& prepend(const char *);
        // _TAG03 Modify - in part

	RJCString& prepend(char);
        // _TAG03 Modify - in part

	RJCString& prepend(long);
        // _TAG03 Modify - in part

	RJCString& prepend(unsigned long);
        // _TAG03 Modify - in part

	RJCString& prepend(double);
		// Prepend the given value to this string.
        // _TAG03 Modify - in part

	RJCString& append(const char *);
        // _TAG03 Modify - in part

	RJCString& append(const char *, int);
        // _TAG03 Modify - in part

	RJCString& append(const RJCString&);
        // _TAG03 Modify - in part

	RJCString& append(const char);
        // _TAG03 Modify - in part

	RJCString& append(const unsigned char);
        // _TAG03 Modify - in part

	RJCString& append(const short);
        // _TAG03 Modify - in part

	RJCString& append(const unsigned short);
        // _TAG03 Modify - in part

	RJCString& append(const int);
        // _TAG03 Modify - in part

	RJCString& append(const unsigned int);
        // _TAG03 Modify - in part

	RJCString& append(const long);
        // _TAG03 Modify - in part

	RJCString& append(const unsigned long);
        // _TAG03 Modify - in part

	RJCString& append(const RTR_I64);
		// _TAG03 Modify - in part 

	RJCString& append(const unsigned RTR_I64);
		// _TAG03 Modify - in part 

	RJCString& append(const float);
        // _TAG03 Modify - in part

	RJCString& append(const double);
		// Append the given value to this string.
        // _TAG03 Modify - in part

	RJCString& appendHex(const char);
        // _TAG03 Modify - in part

	RJCString& appendHex(const unsigned char);
        // _TAG03 Modify - in part

	RJCString& appendHex(const short);
        // _TAG03 Modify - in part

	RJCString& appendHex(const unsigned short);
        // _TAG03 Modify - in part

	RJCString& appendHex(const int);
        // _TAG03 Modify - in part

	RJCString& appendHex(const unsigned int);
        // _TAG03 Modify - in part

	RJCString& appendHex(const long);
		// _TAG03 Modify - in part 

	RJCString& appendHex(const unsigned long);
		// _TAG03 Modify - in part 

	RJCString& appendHex(const RTR_I64);
		// _TAG03 Modify - in part 

	RJCString& appendHex(const unsigned RTR_I64);
		// _TAG03 Modify - in part 
	
	RJCString& toLower();
		// Put this string in lower case.
        // _TAG03 Modify - in part

	RJCString& toUpper();
		// Put this string in upper case.
        // _TAG03 Modify - in part

// Truncate
	void leftAdjust();	
		// Remove leading white-space from this string.
		// _TAG04 Truncate

	void rightAdjust();	
		// Remove trailing white-space from this string.
        // _TAG04 Truncate

	RJCString& head(unsigned int n); 	
		// Trim this string to the first n characters.
		// ENSURE : count() = n \\
		//          head(0) implies isEmpty()
        // _TAG04 Truncate

	RJCString& tail(unsigned int n); 	
		// Trim the first count() - n characters from this string.
		// ENSURE : count() = n \\
		//          tail(0) implies isEmpty()
        // _TAG04 Truncate

// Comparison
 	int compare(const char *) const;
		// Is this string greater than (result == 1), equal to (result == 0), 
		// or less than (result == 1) the given string?
		// _TAG05 Comparison

// Access
	char operator[](int i) const;
		// The i'th character in this string.
		// REQUIRE : i >= lower()
		// REQUIRE : i <= upper()
		// _TAG06 Access

	operator const char *() const;
		// A pointer to the storage for this string. 
		// Result is null terminated ( i.e. result[count()] == '\0'). 
		// Note: Nulls may be imbedded in data.
        // _TAG06 Access

	RJCString subString(int p1, int p2);
		// A new string which characters from positions p1 through p2
		// REQUIRE: p1 >= lower()
		// REQUIRE: p2 <= upper()
		// REQUIRE: p1 <= p2
        // _TAG06 Access

	inline const char *to_c() const;
		// A pointer to the internal storage.
		// Note: unlike use of the cast operator (const char *) storage 
		// is not null terminated by this call.
        // _TAG06 Access

// Query
	RTRBOOL contains(const char *) const;
		// Does this string contain a sub-string equal to the given string?
		// _TAG07 Query

	RTRBOOL contains(const char) const;
		// Does this string contain the given character?
        // _TAG07 Query

	int	indexOf(char c, int p1);
		// The index of the first instance c found in this string after 
		// position p1
		// REQUIRE: p1 >= lower()
		// REQUIRE: p1 <= upper()
		// ENSURE : result >= lower() implies operator[](result) == c
        // _TAG07 Query

// Transform
	int toInteger() const;
		// This string as an integer.
		// _TAG08 Transform

	float toFloat() const;
		// This string as a float.
		// _TAG08 Transform

	double toDouble() const;
		// This string as a double.
        // _TAG08 Transform

	RTRBOOL toBoolean() const;
		// This string as a boolean.
        // _TAG08 Transform

// Operators
	RJCString& operator=(const char *);
		// _TAG09 Operators

	RJCString& operator=(const RJCString&);
		// Assign this string to other string.
        // _TAG09 Operators

	RTRBOOL operator==(const char *) const;
        // _TAG09 Operators

	RTRBOOL operator==(const RJCString&) const;
        // _TAG09 Operators

	RTRBOOL operator!=(const char *) const;
        // _TAG09 Operators

	RTRBOOL operator!=(const RJCString&) const;
        // _TAG09 Operators

	RTRBOOL operator>(const char *) const;
        // _TAG09 Operators

	RTRBOOL operator>(const RJCString&) const;
        // _TAG09 Operators

	RTRBOOL operator>=(const char *) const;
        // _TAG09 Operators

	RTRBOOL operator>=(const RJCString&) const;
        // _TAG09 Operators

	RTRBOOL operator<(const char *) const;
        // _TAG09 Operators

	RTRBOOL operator<(const RJCString&) const;
        // _TAG09 Operators

	RTRBOOL operator<=(const char *) const;
        // _TAG09 Operators

	RTRBOOL operator<=(const RJCString&) const;
		// Compare this string with other
        // _TAG09 Operators

	RJCString& operator+=(const char *);
        // _TAG09 Operators

	RJCString& operator+=(const RJCString&);
        // _TAG09 Operators

	RJCString& operator+=(const char);
		// Append other string or character to this string.
        // _TAG09 Operators

// Operations
	void grow(unsigned int n);
		// Increase the capacity of this string to accomodate n bytes.
		// ENSURE : capacity() >= n
		// _TAG10 Operations

	void trim(unsigned int);
		// Decrease the capacity of this string to accomodate n bytes.
		// ENSURE : capacity() <= n
		//          count() <= n
        // _TAG10 Operations

	void setCount(unsigned int i);
		// Set count to i.
		// [Useful when using the string storage (via to_c()) as a buffer]
		// REQUIRE : i <= capacity()
        // _TAG10 Operations

// Compatiblity - OBSOLETE

	RJCString(const char *str, int n);
		// A string with a copy of the n bytes from null terminated string str.
		// Compatibility of using "int" rather than "unsigned int" \\
		// Compatitbility - OBSOLETE
		// _TAG Constructor

 	RTRBOOL isEqual(const char *) const;
		// Use compare()
        // _TAG11 Compatibility - OBSOLETE

	RJCString& empty();
		// Use clear()
        // _TAG11 Compatibility - OBSOLETE

	inline int length() const;
		// Use count()
        // _TAG11 Compatibility - OBSOLETE

	int	index(char c, int start);
		// Use indexOf
        // _TAG11 Compatibility - OBSOLETE

	RJCString& set(RJCString&, unsigned int p1, unsigned int p2);
        // _TAG11 Compatibility - OBSOLETE

	RJCString& fromInteger(int i);
		// Use fromNumeric
        // _TAG11 Compatibility - OBSOLETE

	void appendNumeric(const char);
        // _TAG11 Compatibility - OBSOLETE

	void appendNumeric(const unsigned char);
        // _TAG11 Compatibility - OBSOLETE

	void appendNumeric(const short);
        // _TAG11 Compatibility - OBSOLETE

	void appendNumeric(const unsigned short);
        // _TAG11 Compatibility - OBSOLETE

	void appendNumeric(const int);
        // _TAG11 Compatibility - OBSOLETE

	void appendNumeric(const unsigned int);
        // _TAG11 Compatibility - OBSOLETE

	void appendNumeric(const long);
        // _TAG11 Compatibility - OBSOLETE

	void appendNumeric(const RTR_I64);
    	// _TAG11 Compatibility - OBSOLETE
	
	void appendNumeric(const unsigned long);
        // _TAG11 Compatibility - OBSOLETE

	void appendNumeric(const float);
        // _TAG11 Compatibility - OBSOLETE

	void appendNumeric(const double);
		// Use append
        // _TAG11 Compatibility - OBSOLETE

protected:
// Methods
	void allocate(unsigned int);
		// Allocate new storage

	void reallocate(unsigned int);
		// Allocate new storage, copying current.

// Data
	char *_area;		// Don't change the order of these!
	unsigned int _capacity;
	unsigned int _count; 
};

inline void RJCString::appendNumeric(const char i) { append(i); };
inline void RJCString::appendNumeric(const unsigned char i) { append(i); };
inline void RJCString::appendNumeric(const short s) { append(s); };
inline void RJCString::appendNumeric(const unsigned short s) { append(s); };
inline void RJCString::appendNumeric(const int i) { append(i); };
inline void RJCString::appendNumeric(const unsigned int i) { append(i); };
inline void RJCString::appendNumeric(const long l) { append(l); };
inline void RJCString::appendNumeric(const RTR_I64 i) { append(i); };
inline void RJCString::appendNumeric(const unsigned long l) { append(l); };
inline void RJCString::appendNumeric(const float f) { append(f); };
inline void RJCString::appendNumeric(const double d) { append(d); };

inline RJCString& RJCString::clear()
{
	_count = 0;
	_area[_count] = '\0';

	RTPOSTCONDITION ( isEmpty() );
	return *this;
}

inline const char *RJCString::to_c() const
{
		// Don't null terminate here for backwards compatibility
		// Clients may have been using this as a "buffer" and storing
		// data without setting _count.
	return _area;
}


inline RJCString& RJCString::empty()
{
	clear();
	return *this;
}

inline int RJCString::length() const
{
	return count();
}

inline const unsigned int RJCString::capacity()const
{
	return _capacity;
}

inline unsigned int RJCString::count() const
{
	return _count;
}

inline RTRBOOL RJCString::isEmpty() const
{
	return count() == 0;
}


inline int RJCString::lower() const
{
	return 1;
}

inline int RJCString::upper() const
{
	return _count;
}


#endif // _rjcstring_h
