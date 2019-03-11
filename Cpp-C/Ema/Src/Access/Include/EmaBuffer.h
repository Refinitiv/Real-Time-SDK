/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_EmaBuffer_h
#define __thomsonreuters_ema_access_EmaBuffer_h

/**
	@class thomsonreuters::ema::access::EmaBuffer EmaBuffer.h "Access/Include/EmaBuffer.h"
	@brief EmaBuffer represents a general use binary buffer.

	EmaBuffer is a buffer of 8 bit long characters where each character is represented by char or byte.
	
	\remark EmaBuffer class contains a copy of the passed in buffer.
	\remark All methods in this class are \ref SingleThreaded.
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {
	
namespace ema {

namespace access {

class CastingOperatorContext;

class EMA_ACCESS_API EmaBuffer
{
public :

	///@name Constructor
	//@{
	/** Constructs EmaBuffer.
	*/
	EmaBuffer();

	/** Assignment constructor
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory containing copied in buffer
		@param[in] length specifies number of characters to copy
	*/
	EmaBuffer( const char* buf, UInt32 length );

	/** Copy constructor.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf copied in EmaBuffer object
	*/
	EmaBuffer( const EmaBuffer& buf );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~EmaBuffer();
	//@}

	///@name Operations
	//@{
	/** Clears contained buffer.
		@return reference to this object
	*/
	EmaBuffer& clear();

	/** Assignment operator.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf copied in EmaBuffer object
		@return reference to this object
	*/
	EmaBuffer& operator=( const EmaBuffer& buf );

	/** Method to set Buffer.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory are containing copied in buffer
		@param[in] length specifies number of characters to copy
		@return reference to this object
	*/
	EmaBuffer& setFrom( const char* buf, UInt32 length );

	/** method to append this object with the passed in EmaBuffer object
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf EmaBuffer to append to this object
		@return reference to this object
	*/
	EmaBuffer& append( const EmaBuffer& buf );

	/** method to append this object with the passed in char
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] c character to append to this object
		@return reference to this object
	*/
	EmaBuffer& append( char c );

	/** method to append this object with the passed in char buffer
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory containing appended buffer
		@param[in] length specifies number of characters to append
		@return reference to this object
	*/
	EmaBuffer& append( const char* buf, UInt32 length );

	/** method to append this object with the passed in EmaBuffer object
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf EmaBuffer to append to this object
		@return reference to this object
	*/
	EmaBuffer& operator+=( const EmaBuffer& buf );
	
	/** method to append this object with the passed in char
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] c character to append to this object
		@return reference to this object
	*/
	EmaBuffer& operator+=( char );

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
	const char* c_buf() const;
	
	/** Returns length of the internal storage memory.
		@return length of the internal buffer
	*/
	UInt32 length() const;

	/** Returns an alphanumeric null-terminated hexadecimal string representation.
	@return const char pointer used for printing out content of the internal buffer to screen
	*/
	const char* asRawHexString() const;

	/** Returns an alphanumeric null-terminated hexadecimal string representation via conversion to operator const char*.
	@return const char pointer used for printing out content of the internal buffer to screen
	*/
	operator const char* () const;

	/** Compare operator.
		@param[in] buf compared EmaBuffer object
		@return true if this and passed in object match
	*/
	bool operator==( const EmaBuffer& buf ) const;

	/** read only index operator
		@throw OmmOutOfRangeException if passed in index is greater than the length of the contained buffer
		@param[in] index specifies position to read
		@return byte at the specified position
	*/
	char operator[]( UInt32 index ) const;
	//@}

protected:

	void markDirty() const;

	char*				_pBuffer;
	UInt32				_length;
	UInt32				_capacity;
	mutable CastingOperatorContext* _pCastingOperatorContext;
};
	
/** addition operator; allows to do a = b + c; operation on EmaBuffer objects
	@param[in] buff_1 first argument of the addition
	@param[in] buff_2 second argument of the addition
	@return copy of EmaBuffer containing both buff_1 and buff_2 in that order
*/
EmaBuffer operator+( EmaBuffer buff_1, const EmaBuffer& buff_2 );

}

}

}

#endif // __thomsonreuters_ema_access_EmaBuffer_h
