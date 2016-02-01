/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_eoabuffer_h
#define __thomsonreuters_eoa_foundation_eoabuffer_h

/**
	@class thomsonreuters::eoa::foundation::EoaBuffer EoaBuffer.h "Foundation/Include/EoaBuffer.h"
	@brief EoaBuffer represents a general use binary buffer.

	EoaBuffer is a buffer of 8 bit long characters where each character is represented by char or byte.
	
	\remark EoaBuffer class contains a copy of the passed in buffer.
	\remark All methods in this class are \ref SingleThreaded.
*/

#include "Foundation/Include/Common.h"

namespace thomsonreuters {
	
namespace eoa {

namespace foundation {

class EOA_FOUNDATION_API EoaBuffer
{
public :

	///@name Constructor
	//@{
	/** Constructs EoaBuffer.
	*/
	EoaBuffer();

	/** Assignment constructor
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory containing copied in buffer
		@param[in] length specifies number of characters to copy
	*/
	EoaBuffer( const char* buf, UInt32 length );

	/** Copy constructor.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf copied in EoaBuffer object
	*/
	EoaBuffer( const EoaBuffer& buf );

	/** Move Constructor
		@param[in] other EoaBuffer object from whom data is moved out
	*/
	EoaBuffer( EoaBuffer&& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~EoaBuffer();
	//@}

	///@name Operations
	//@{
	/** Clears contained buffer.
		@return reference to this object
	*/
	EoaBuffer& clear() throw();

	/** Assignment operator.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf copied in EoaBuffer object
		@return reference to this object
	*/
	EoaBuffer& operator=( const EoaBuffer& buf );

	/** Assignment move operator.
		@param[in] buf moved in EoaBuffer object
		@return reference to this object
	*/
	EoaBuffer& operator=( EoaBuffer&& buf ) throw();

	/** Method to set Buffer.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory are containing copied in buffer
		@param[in] length specifies number of characters to copy
		@return reference to this object
	*/
	EoaBuffer& setFrom( const char* buf, UInt32 length );

	/** method to append this object with the passed in EoaBuffer object
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf EoaBuffer to append to this object
		@return reference to this object
	*/
	EoaBuffer& append( const EoaBuffer& buf );

	/** method to append this object with the passed in char
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] c character to append to this object
		@return reference to this object
	*/
	EoaBuffer& append( char c );

	/** method to append this object with the passed in char buffer
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory containing appended buffer
		@param[in] length specifies number of characters to append
		@return reference to this object
	*/
	EoaBuffer& append( const char* buf, UInt32 length );

	/** method to append this object with the passed in EoaBuffer object
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf EoaBuffer to append to this object
		@return reference to this object
	*/
	EoaBuffer& operator+=( const EoaBuffer& buf );
	
	/** method to append this object with the passed in char
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] c character to append to this object
		@return reference to this object
	*/
	EoaBuffer& operator+=( char );

	/** read write index operator
		@throw OmmOutOfRangeException if passed in index is greater than the length of the contained buffer
		@param[in] index specifies position to read or write
		@return reference to the byte at the specified position
	*/
	char& operator[]( UInt32 index );
	//@}

	///@name Accessors
	//@{
	/** Returns pointer to the internal storage memory
		@return pointer to the internal memory area containing buffer data
	*/
	const char* c_buf() const throw();
	
	/** Returns length of the internal storage memory.
		@return length of the internal buffer
	*/
	UInt32 length() const throw();

	/** Returns an alphanumeric null-terminated hexadecimal string representation via conversion to operator const char*.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@return const char pointer used for printing out content of the internal buffer to screen
	*/
	operator const char* () const;

	/** Compare operator.
		@param[in] buf compared EoaBuffer object
		@return true if this and passed in object match
	*/
	bool operator==( const EoaBuffer& buf ) const throw();

	/** read only index operator
		@throw OmmOutOfRangeException if passed in index is greater than the length of the contained buffer
		@param[in] index specifies position to read
		@return byte at the specified position
	*/
	char operator[]( UInt32 index ) const;
	//@}

protected:

	char*				_pBuffer;
	UInt32				_length;
	UInt32				_capacity;
	mutable void*		_pToString;
	mutable bool		_dirty;
};
	
/** addition operator; allows to do a = b + c; operation on EoaBuffer objects
	@throw OmmMemoryExhaustionException if application runs out of memory
	@param[in] buff_1 first argument of the addition
	@param[in] buff_2 second argument of the addition
	@return copy of EoaBuffer containing both buff_1 and buff_2 in that order
*/
EoaBuffer operator+( EoaBuffer buff_1, const EoaBuffer& buff_2 );

}

}

}

#endif // __thomsonreuters_eoa_foundation_eoabuffer_h
