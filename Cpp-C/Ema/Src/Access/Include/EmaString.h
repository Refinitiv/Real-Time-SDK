/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_EmaString_h
#define __refinitiv_ema_access_EmaString_h

/**
	@class refinitiv::ema::access::EmaString EmaString.h "Access/Include/EmaString.h"
	@brief EmaString class is a container of a null terminated Ascii character string.

	The following code snippet shows simple usage of EmaString;

	\code

	EmaString myString( "ABCDEFG" );

	Int32 position = myString.find( "DE" );         // Returns 3

	position = myString.find("XYZ");                // Returns EmaString::npos (-1)
	
	...

	EmaString temp = myString.substr( 3, 4 )        // Returns "DEFG"

	temp = myString.substr( 0, EmaString::npos );   // Returns "ABCDEFG"

	temp = myString.substr( 4, EmaString::npos );   // Returns "EFG"
	
	...

	myString.append( ( Int32 ) 4 );

	std::cout << myString << endl;                  // Prints "ABCDEFG4"

	...

	\endcode

	\remark EmaString class contains a copy of the passed in string.
	\remark All methods in this class are \ref SingleThreaded.
*/

#include "Access/Include/Common.h"

namespace refinitiv {
	
namespace ema {

namespace access {

class EMA_ACCESS_API EmaString
{
public :

	/** npos. Represents the greatest possible value of type unsigned int
		\remark npos is often used as a length parameter in EmaString methods to mean "until the end of the string"
		\remark When used as a return value, npos indicates that the element was "not found"
	*/
	static const UInt32 npos = -1;

	///@name Constructor
	//@{
	/** Constructs EmaString.
	*/
	EmaString();

	/** Copy constructor.
		\remark Preallocates memory of passed in length if used as in EmaString( 0, 1000 );
		\remark If length parameter is set to 0 (zero), then the actual length is determined 
			looking up for a null character (e.g. strlen()).
		@throw OmmMemoryExhaustionException if app runs out of memory
		@throw OmmInvalidUsageException if passed in string is longer than MAX_UINT32
		@param[in] str pointer to the memory containing copied in character string
		@param[in] length specifies number of characters to copy
	*/
	EmaString( const char* str, UInt32 length = EmaString::npos );

	/** Copy constructor.
		@param[in] other copied in EmaString object
	*/
	EmaString( const EmaString& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~EmaString();
	//}@

	///@name Operations
	//@{
	/** Clears the contained string.
		@return reference to this object
	*/
	EmaString& clear();

	/** Set method. This method copies "length" number of characters from the "str" pointer.
		\remark Preallocates empty memory of passed in length if used as follows EmaString::set( 0, 1000 );
		@throw OmmMemoryExhaustionException if app runs out of memory
		@throw OmmInvalidUsageException if passed in string is longer than MAX_UINT32
		@param[in] str pointer to the memory containing copied in character string
		@param[in] length specifies number of characters to copy
		@return reference to this object
	*/
	EmaString& set( const char* str, UInt32 length = EmaString::npos );

	/** Assignment operator
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] other copied in EmaString object
		@return reference to this object
	*/
	EmaString& operator=( const EmaString& other );

	/** Assignment operator
		\remark a null character determines length of the copied in string
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] other copied in character string
		@return reference to this object
	*/
	EmaString& operator=( const char* other );

	/** Append method. Appends string representation of passed in Int64
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& append( Int64 value );

	/** Append method. Appends string representation of passed in UInt64
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& append( UInt64 value );

	/** Append method. Appends string representation of passed in Int32 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& append( Int32 value );

	/** Append method. Appends string representation of passed in UInt32
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& append( UInt32 value );

	/** Append method. Appends string representation of passed in float 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& append( float value );

	/** Append method. Appends string representation of passed in double 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& append( double value );

	/** Append method. Appends string representation of passed in const char* 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& append( const char* value );

	/** Append method. Appends string representation of passed in EmaString 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& append( const EmaString& value );

	/** Append operator. Appends string representation of passed in Int64
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& operator+=( Int64 value );

	/** Append operator. Appends string representation of passed in UInt64
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& operator+=( UInt64 value );

	/** Append operator. Appends string representation of passed in Int32
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& operator+=( Int32 value );

	/** Append operator. Appends string representation of passed in UInt32
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& operator+=( UInt32 value );

	/** Append operator. Appends string representation of passed in float
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& operator+=( float value );

	/** Append operator. Appends string representation of passed in double
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& operator+=( double value );

	/** Append operator. Appends string representation of passed in const char* 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& operator+=( const char* value );

	/** Append operator. Appends string representation of passed in EmaString 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EmaString& operator+=( const EmaString& value );

	/** Addition operator. Allows a = b + c;
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] str specifies EmaString to be added to this EmaString
		@return result of addition
	*/
	EmaString operator+( const EmaString& str ) const;

	/** Addition operator. Allows a = b + c;
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] str specifies char string to be added to this EmaString
		@return result of addition
	*/
	EmaString operator+( const char* str ) const;

	/** read write index operator
		@throw OmmOutOfRangeException if passed in index is greater than the length of the contained buffer
		@param[in] index specifies position of a character to be read
		@return reference to character at the specified position
	*/
	char& operator[]( UInt32 index );

	/** removes white spaces from the front and back of the string.
		@return reference to this object
	*/
	EmaString& trimWhitespace();
	//@}

	///@name Accessors
	//@{
	/** check if EmaString is empty
		@return true if string is empty; false otherwise
	*/
	bool empty() const;

	/** method to get pointer to internal null terminated string of characters
		@return const char pointer used for printing out content of the internal string to screen
	*/
	virtual const char* c_str() const;

	/** Returns length of the contained string. Null termination is not counted in.
		@return length
	*/
	UInt32 length() const;

	/** read only index operator
		@throw OmmOutOfRangeException if passed in index is greater than the length of the contained buffer
		@param[in] index specifies position of a character to be read
		@return character at the specified position
	*/
	char operator[]( UInt32 index ) const;

	/** sub string operator
		@throw OmmOutOfRangeException if passed in index and length resolve to out of range
		@param[in] index specifies starting position of the sub string
		@param[in] length specifies length of the sub string
		@return copy of sub string
	*/
	EmaString substr( UInt32 index, UInt32 length ) const;

	/** Conversion operator to const char*
		\remark invokes c_str()
		@return const char pointer used for printing out content of the internal string to screen
	*/
	operator const char* () const;

	/** Method to find occurrence of the passed in string.
		\remark This is a case sensitive method.
		@param[in] str specifies looked up string
		@param[in] index specifies position to start the search from
		@return position where matching string starts; if not found -1
	*/
	Int32 find( const EmaString& str, Int32 index = 0 ) const;

	/** Method to find occurrence of the passed in string.
		\remark This is a case sensitive method.
		@param[in] str specifies looked up null terminated string of characters
		@param[in] index specifies position to start the search from
		@return position where matching string starts; if not found -1
	*/
	Int32 find( const char* str, Int32 index = 0 ) const;

	/** Method to find occurrence of the passed in string starting from the end
		\remark This is a case sensitive method.
		@param[in] str specifies looked up string
		@return position where matching string starts; if not found -1
	*/
	Int32 findLast( const EmaString& str ) const;

	/** Method to find occurrence of the passed in string from the end
		\remark This is a case sensitive method.
		@param[in] str specifies looked up null terminated string of characters
		@return position where matching string starts; if not found -1
	*/
	Int32 findLast( const char* ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object and passed in match; false otherwise
	*/
	bool operator==( const EmaString& str ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object and passed in do not match; false otherwise
	*/
	bool operator!=( const EmaString& str ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object is greater than the passed in; false otherwise
	*/
	bool operator>( const EmaString& str ) const;	

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object is less than the passed in; false otherwise
	*/
	bool operator<( const EmaString& str ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object is greater than or equal to the passed in; false otherwise
	*/
	bool operator>=( const EmaString& str ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object is less than or equal to the passed in; false otherwise
	*/
	bool operator<=( const EmaString& str ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object and passed in match; false otherwise
	*/
	bool operator==( const char* str ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object and passed in do not match; false otherwise
	*/
	bool operator!=( const char* str ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object is greater than the passed in; false otherwise
	*/
	bool operator>( const char* str ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object is less than the passed in; false otherwise
	*/
	bool operator<( const char* str ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object is greater than or equal to the passed in; false otherwise
	*/
	bool operator>=( const char* str ) const;

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object is less than or equal the passed in; false otherwise
	*/
	bool operator<=( const char* str ) const;

	/** Case insensitive comparison operator
		@param[in] str specifies string to match against this object
		@return true if this object and passed in match; false otherwise
	*/
	bool caseInsensitiveCompare( const EmaString& str ) const;

	/** Case insensitive comparison operator
		@param[in] str specifies null terminated string to match against this object
		@return true if this object and passed in match; false otherwise
	*/
	bool caseInsensitiveCompare( const char * str ) const;
	//@}

protected:

	mutable char*	_pString;
	UInt32			_length;
	mutable UInt32	_capacity;

private:

    int compare( const char * rhs ) const;
};

}

}

}

#endif //__refinitiv_ema_access_EmaString_h
