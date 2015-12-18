/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_eoabufferu16_h
#define __thomsonreuters_eoa_foundation_eoabufferu16_h

/**
	@class thomsonreuters::eoa::foundation::EoaBufferU16 EoaBufferU16.h "Foundation/Include/EoaBufferU16.h"
	@brief EoaBufferU16 represents general use binary buffer.

	EoaBufferU16 is a buffer of 16 bit long characters where each character is represented by UInt16.
	EoaBufferU16 is used to contain UTF16 data.

	\remark EoaBufferU16 class contains a copy of the buffer passed on set methods.
	\remark All methods in this class are \ref SingleThreaded.
*/

#include "Foundation/Include/Common.h"

namespace thomsonreuters {
	
namespace eoa {

namespace foundation {

class EOA_FOUNDATION_API EoaBufferU16
{
public :

    ///@name Constructor
    //@{
    /** Constructs EoaBufferU16
    */
	EoaBufferU16();

	/** Assignment constructor
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory containing copied in buffer
		@param[in] length specifies number of characters to copy
	*/
	EoaBufferU16( const UInt16* buf, UInt32 length );

	/** Copy constructor.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf copied in EoaBufferU16 object
	*/
	EoaBufferU16( const EoaBufferU16& buf );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~EoaBufferU16();
	//@}

	///@name Operations
	//@{
	/** Clears contained buffer.
		@return reference to this object
	*/
	EoaBufferU16& clear();

	/** Assignment operator.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf copied in EoaBufferU16 object
		@return reference to this object
	*/
	EoaBufferU16& operator=( const EoaBufferU16& buf );

	/** Method to set Buffer.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory are containing copied in buffer
		@param[in] length specifies number of characters to copy
		@return reference to this object
	*/
	EoaBufferU16& setFrom( const UInt16* buf, UInt32 length );

	/** method to append this object with the passed in EoaBufferU16 object
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf EoaBufferU16 to append to this object
		@return reference to this object
	*/
	EoaBufferU16& append( const EoaBufferU16& buf );

	/** method to append this object with the passed in 16 byte long character
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] c character to append to this object
		@return reference to this object
	*/
	EoaBufferU16& append( UInt16 c );

	/** method to append this object with the passed in char buffer
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory containing appended buffer
		@param[in] length specifies number of characters to append
		@return reference to this object
	*/
	EoaBufferU16& append( const UInt16* buf, UInt32 length );

	/** method to append this object with the passed in EoaBufferU16 object
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf EmaBuffer to append to this object
		@return reference to this object
	*/
	EoaBufferU16& operator+=( const EoaBufferU16& buf );
	
	/** method to append this object with the passed in 16 byte long char
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] c character to append to this object
		@return reference to this object
	*/
	EoaBufferU16& operator+=( UInt16 );

	/** read write index operator
		@throw OmmOutOfRangeException if passed in index is greater than the length of the contained buffer
		@param[in] index specifies position to read or write
		@return reference to the byte at the specified position
	*/
	UInt16& operator[]( UInt32 index );
	//@}

	///@name Accessors
	//@{
	/** Returns pointer to the internal storage memory
		@return pointer to the internal memory area containing buffer data
	*/
	const UInt16* u16_buf() const;
	
	/** Returns length of the internal storage memory.
		@return length of the internal buffer
	*/
	UInt32 length() const;

	/** Compare operator.
		@param[in] buf compared EoaBufferU16 object
		@return true if this and passed in object match
	*/
	bool operator==( const EoaBufferU16& buf ) const;

	/** read only index operator
		@throw OmmOutOfRangeException if passed in index is greater than the length of the contained buffer
		@param[in] index specifies position to read
		@return byte at the specified position
	*/
	UInt16 operator[]( UInt32 index ) const;
	//@}

protected:

	UInt16*				_pBuffer;
	UInt32				_length;
	UInt32				_capacity;
};
	
/** addition operator; allows to do a = b + c; operation on EoaBufferU16 objects
	@param[in] buff_1 first argument of the addition
	@param[in] buff_2 second argument of the addition
	@return copy of EmaBuffer containing both buff_1 and buff_2 in that order
*/
EoaBufferU16 operator+( EoaBufferU16 buff_1, const EoaBufferU16& buff_2 );

}

}

}

#endif // __thomsonreuters_eoa_foundation_eoabufferu16_h
