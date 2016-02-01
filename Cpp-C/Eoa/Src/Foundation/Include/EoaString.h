/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_eoastring_h
#define __thomsonreuters_eoa_foundation_eoastring_h

/**
	@class thomsonreuters::eoa::foundation::EoaString EoaString.h "Foundation/Include/EoaString.h"
	@brief EoaString class is a container of a null terminated Ascii character string.

	The following code snippet shows simple usage of EoaString;

	\code

	EoaString myString( "ABCDEFG" );

	Int32 position = myString.find( "DE" );         // Returns 3

	position = myString.find("XYZ");                // Returns EoaString::npos (-1)
	
	...

	EoaString temp = myString.substr( 3, 4 )        // Returns "DEFG"

	temp = myString.substr( 0, EoaString::npos );   // Returns "ABCDEFG"

	temp = myString.substr( 4, EoaString::npos );   // Returns "EFG"
	
	...

	myString.append( ( Int32 ) 4 );

	std::cout << myString << endl;                  // Prints "ABCDEFG4"

	...

	\endcode

	\remark EoaString class contains a copy of the passed in string.
	\remark All methods in this class are \ref SingleThreaded.
*/

#include "Foundation/Include/Common.h"

namespace thomsonreuters {
	
namespace eoa {

namespace foundation {

class EOA_FOUNDATION_API EoaString
{
public :

	/** npos. Represents the greatest possible value of type unsigned int
		\remark npos is often used as a length parameter in EoaString methods to mean "until the end of the string"
		\remark When used as a return value, npos indicates that the element was "not found"
	*/
	static const unsigned int npos = -1;

	///@name Constructor
	//@{
	/** Constructs empty EoaString.
	*/
	EoaString();

	/** Copy constructor.
		\remark Preallocates memory of passed in length if used as in EoaString( 0, 1000 );
		\remark If length parameter is set to 0 (zero), then the actual length is determined 
			looking up for a null character (e.g. strlen()).
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] str pointer to the memory containing copied in character string
		@param[in] length specifies number of characters to copy
	*/
	EoaString( const char* str, UInt32 length = EoaString::npos );

	/** Copy constructor.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] other copied in EoaString object
	*/
	EoaString( const EoaString& other );

	/** Move Constructor
		@param[in] other EoaString object from whom text is moved out
	*/
	EoaString( EoaString&& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~EoaString();
	//}@

	///@name Operations
	//@{
	/** Clears the contained string.
		@return reference to this object
	*/
	EoaString& clear() throw();

	/** Set method. This method copies "length" number of characters from the "str" pointer.
		\remark Preallocates empty memory of passed in length if used as follows EoaString::set( 0, 1000 );
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] str pointer to the memory containing copied in character string
		@param[in] length specifies number of characters to copy
		@return reference to this object
	*/
	EoaString& set( const char* str, UInt32 length = EoaString::npos );

	/** Assignment operator
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] other copied in EoaString object
		@return reference to this object
	*/
	EoaString& operator=( const EoaString& other );

	/** Assignment move operator
		@param[in] other moved in EoaString object
		@return reference to this object
	*/
	EoaString& operator=( EoaString&& other ) throw();

	/** Assignment operator
		\remark a null character determines length of the copied in string
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] other copied in character string
		@return reference to this object
	*/
	EoaString& operator=( const char* other );

	/** Append method. Appends string representation of passed in Int64
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& append( Int64 value );

	/** Append method. Appends string representation of passed in UInt64
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& append( UInt64 value );

	/** Append method. Appends string representation of passed in Int32 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& append( Int32 value );

	/** Append method. Appends string representation of passed in UInt32
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& append( UInt32 value );

	/** Append method. Appends string representation of passed in float 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& append( float value );

	/** Append method. Appends string representation of passed in double 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& append( double value );

	/** Append method. Appends string representation of passed in const char* 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& append( const char* value );

	/** Append method. Appends string representation of passed in EoaString 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& append( const EoaString& value );

	/** Append operator. Appends string representation of passed in Int64
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& operator+=( Int64 value );

	/** Append operator. Appends string representation of passed in UInt64
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& operator+=( UInt64 value );

	/** Append operator. Appends string representation of passed in Int32
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& operator+=( Int32 value );

	/** Append operator. Appends string representation of passed in UInt32
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& operator+=( UInt32 value );

	/** Append operator. Appends string representation of passed in float
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& operator+=( float value );

	/** Append operator. Appends string representation of passed in double
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& operator+=( double value );

	/** Append operator. Appends string representation of passed in const char* 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& operator+=( const char* value );

	/** Append operator. Appends string representation of passed in EoaString 
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] value to be appended to this object
		@return reference to this object
	*/
	EoaString& operator+=( const EoaString& value );

	/** Addition operator. Allows a = b + c;
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] str specifies EoaString to be added to this EoaString
		@return result of addition
	*/
	EoaString operator+( const EoaString& str );

	/** Addition operator. Allows a = b + c;
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] str specifies char string to be added to this EoaString
		@return result of addition
	*/
	EoaString operator+( const char* str );

	/** read write index operator
		@throw OmmOutOfRangeException if passed in index is greater than the length of the contained buffer
		@param[in] index specifies position of a character to be read
		@return reference to character at the specified position
	*/
	char& operator[]( UInt32 index );

	/** removes white spaces from the front and back of the string.
		@return reference to this object
	*/
	EoaString& trimWhitespace() throw();
	//@}

	///@name Accessors
	//@{
	/** check if EoaString is empty
		@return true if string is empty; false otherwise
	*/
	bool empty() const throw();

	/** method to get pointer to internal null terminated string of characters
		@throw OmmMemoryExhaustionException if application runs out of memory
		@return const char pointer used for printing out content of the internal string to screen
	*/
	virtual const char* c_str() const;

	/** Returns length of the contained string. Null termination is not counted in.
		@return length
	*/
	UInt32 length() const throw();

	/** read only index operator
		@throw OmmOutOfRangeException if passed in index is greater than the length of the contained buffer
		@param[in] index specifies position of a character to be read
		@return character at the specified position
	*/
	char operator[]( UInt32 index ) const;

	/** sub string operator
		@throw OmmOutOfRangeException if passed in index and length resolve to out of range
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] index specifies starting position of the sub string
		@param[in] length specifies length of the sub string
		@return copy of sub string
	*/
	EoaString substr( UInt32 index, UInt32 length ) const;

	/** Conversion operator to const char*
		\remark invokes c_str()
		@throw OmmMemoryExhaustionException if application runs out of memory
		@return const char pointer used for printing out content of the internal string to screen
	*/
	operator const char* () const;

	/** Method to find occurrence of the passed in string.
		\remark This is a case sensitive method.
		@param[in] str specifies looked up string
		@param[in] index specifies position to start the search from
		@return position where matching string starts; if not found -1
	*/
	Int32 find( const EoaString& str, Int32 index = 0 ) const throw();

	/** Method to find occurrence of the passed in string.
		\remark This is a case sensitive method.
		@param[in] str specifies looked up null terminated string of characters
		@param[in] index specifies position to start the search from
		@return position where matching string starts; if not found -1
	*/
	Int32 find( const char* str, Int32 index = 0 ) const throw();

	/** Method to find occurrence of the passed in string starting from the end
		\remark This is a case sensitive method.
		@param[in] str specifies looked up string
		@return position where matching string starts; if not found -1
	*/
	Int32 findLast( const EoaString& str ) const throw();

	/** Method to find occurrence of the passed in string from the end
		\remark This is a case sensitive method.
		@param[in] str specifies looked up null terminated string of characters
		@return position where matching string starts; if not found -1
	*/
	Int32 findLast( const char* ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object and passed in match; false otherwise
	*/
	bool operator==( const EoaString& str ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object and passed in do not match; false otherwise
	*/
	bool operator!=( const EoaString& str ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object is greater than the passed in; false otherwise
	*/
	bool operator>( const EoaString& str ) const throw();	

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object is less than the passed in; false otherwise
	*/
	bool operator<( const EoaString& str ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object is greater than or equal to the passed in; false otherwise
	*/
	bool operator>=( const EoaString& str ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies string to match against this object
		@return true if this object is less than or equal to the passed in; false otherwise
	*/
	bool operator<=( const EoaString& str ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object and passed in match; false otherwise
	*/
	bool operator==( const char* str ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object and passed in do not match; false otherwise
	*/
	bool operator!=( const char* str ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object is greater than the passed in; false otherwise
	*/
	bool operator>( const char* str ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object is less than the passed in; false otherwise
	*/
	bool operator<( const char* str ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object is greater than or equal to the passed in; false otherwise
	*/
	bool operator>=( const char* str ) const throw();

	/** Compare operator
		\remark This operator is case sensitive.
		@param[in] str specifies null terminated string to match against this object
		@return true if this object is less than or equal the passed in; false otherwise
	*/
	bool operator<=( const char* str ) const throw();

	/** Case insensitive comparison operator
		@param[in] str specifies string to match against this object
		@return true if this object and passed in match; false otherwise
	*/
	bool caseInsensitiveCompare( const EoaString& str ) const throw();

	/** Case insensitive comparison operator
		@param[in] str specifies null terminated string to match against this object
		@return true if this object and passed in match; false otherwise
	*/
	bool caseInsensitiveCompare( const char* str ) const throw();
	//@}

protected:

	mutable char*	_pString;
	UInt32			_length;
	mutable UInt32	_capacity;

private:

    int compare( const char* rhs ) const throw();
};

}

}

}

#endif //__thomsonreuters_eoa_foundation_eoastring_h
