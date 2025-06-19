/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_EmaBufferU16_h
#define __refinitiv_ema_access_EmaBufferU16_h

/**
	@class refinitiv::ema::access::EmaBufferU16 EmaBufferU16.h "Access/Include/Include/EmaBufferU16.h"
	@brief EmaBufferU16 represents general use binary buffer.

	EmaBufferU16 is a buffer of 16 bit long characters where each character is represented by UInt16.
	EmaBufferU16 is used to contain UTF16 data.

	\remark EmaBufferU16 class contains a copy of the buffer passed on set methods.
	\remark All methods in this class are \ref SingleThreaded.
*/

#include "Access/Include/Common.h"

namespace refinitiv {
	
namespace ema {

namespace access {

class EMA_ACCESS_API EmaBufferU16
{
public :

    ///@name Constructor
    //@{
    /** Constructs EmaBufferU16
    */
	EmaBufferU16();

	/** Assignment constructor
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory containing copied in buffer
		@param[in] length specifies number of characters to copy
	*/
	EmaBufferU16( const UInt16* buf, UInt32 length );

	/** Copy constructor.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf copied in EmaBufferU16 object
	*/
	EmaBufferU16( const EmaBufferU16& buf );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~EmaBufferU16();
	//@}

	///@name Operations
	//@{
	/** Clears contained buffer.
		@return reference to this object
	*/
	EmaBufferU16& clear();

	/** Assignment operator.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf copied in EmaBufferU16 object
		@return reference to this object
	*/
	EmaBufferU16& operator=( const EmaBufferU16& buf );

	/** Method to set Buffer.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory are containing copied in buffer
		@param[in] length specifies number of characters to copy
		@return reference to this object
	*/
	EmaBufferU16& setFrom( const UInt16* buf, UInt32 length );

	/** method to append this object with the passed in EmaBufferU16 object
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf EmaBufferU16 to append to this object
		@return reference to this object
	*/
	EmaBufferU16& append( const EmaBufferU16& buf );

	/** method to append this object with the passed in 16 byte long character
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] c character to append to this object
		@return reference to this object
	*/
	EmaBufferU16& append( UInt16 c );

	/** method to append this object with the passed in char buffer
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf pointer to the memory containing appended buffer
		@param[in] length specifies number of characters to append
		@return reference to this object
	*/
	EmaBufferU16& append( const UInt16* buf, UInt32 length );

	/** method to append this object with the passed in EmaBufferU16 object
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] buf EmaBuffer to append to this object
		@return reference to this object
	*/
	EmaBufferU16& operator+=( const EmaBufferU16& buf );
	
	/** method to append this object with the passed in 16 byte long char
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] c character to append to this object
		@return reference to this object
	*/
	EmaBufferU16& operator+=( UInt16 );

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
		@param[in] buf compared EmaBufferU16 object
		@return true if this and passed in object match
	*/
	bool operator==( const EmaBufferU16& buf ) const;

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
	
/** addition operator; allows to do a = b + c; operation on EmaBufferU16 objects
	@param[in] buff_1 first argument of the addition
	@param[in] buff_2 second argument of the addition
	@return copy of EmaBuffer containing both buff_1 and buff_2 in that order
*/
EmaBufferU16 operator+( EmaBufferU16 buff_1, const EmaBufferU16& buff_2 );

}

}

}

#endif // __refinitiv_ema_access_EmaBufferU16_h
