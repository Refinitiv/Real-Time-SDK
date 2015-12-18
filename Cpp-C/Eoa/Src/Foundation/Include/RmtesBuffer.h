/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_rmtesbuffer_h
#define __thomsonreuters_eoa_foundation_rmtesbuffer_h

/**
	@class thomsonreuters::eoa::foundation::RmtesBuffer RmtesBuffer.h "Foundation/Include/RmtesBuffer.h"
	@brief RmtesBuffer represents RMTES data.

	RmtesBuffer stores and applies RMTES data.

	\remark RmtesBuffer class contains a copy of the buffer passed on apply methods.
	\remark All methods in this class are \ref SingleThreaded.
*/

#include "Foundation/Include/Common.h"

namespace thomsonreuters{

namespace eoa {

namespace foundation {

class EoaBuffer;
class EoaString;
class EoaBufferU16;

class EOA_FOUNDATION_API RmtesBuffer
{
public :

	///@name Constructor
	//@{
	/** Constructs RmtesBuffer.
	*/
	RmtesBuffer();

	/** Constructs RmtesBuffer.
		\remark Preallocates memory if length is different than 0
		@throw OmmMemoryExhaustionException if application runs out of memory
		@param[in] length specifies size of preallocated memory
	*/
	RmtesBuffer( UInt32 length );

	/** Copy constructor.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@throw OmmInvalidUsageException if fails to apply passed in content
		@param[in] buf copied in RmtesBuffer object
	*/
	RmtesBuffer( const RmtesBuffer& buf );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~RmtesBuffer();
	//@}

	///@name Accessors
	//@{
	/** Returns the content converted as UTF8.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@throw OmmInvalidUsageException if fails to convert
		@return EmaBuffer containing RMTES data converted to UTF8
	*/
	const EoaBuffer& getAsUTF8() const;

	/** Returns the content converted as UTF16.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@throw OmmInvalidUsageException if fails to convert
		@return EmaBufferU16 containing RMTES data converted to UTF16
	*/
	const EoaBufferU16& getAsUTF16() const;

	/** Returns a string representation of the class instance which is converted to UTF8.
		@throw OmmMemoryExhaustionException if application runs out of memory
		@throw OmmInvalidUsageException if fails to convert
		@return EmaString containing RMTES data converted to UTF8
	*/
	const EoaString& toString() const;
	//@}

	///@name Operations
	//@{
	/** Clears contained content.
		@return reference to this object
	*/
	RmtesBuffer& clear();

	/** apply passed in RMTES data
		@throw OmmMemoryExhaustionException if application runs out of memory
		@throw OmmInvalidUsageException if fails to apply
		@param[in] buf specifies RmtesBuffer to be applied to this object
		@return reference to this object
	*/
	RmtesBuffer& apply( const RmtesBuffer& buf );
	//@}

private :

	friend class Tag;
	friend class LeafDecoder;
	friend class CacheLeaf;

	void setHexData( const char* , UInt32 );

	UInt64				_rmtesSpace[24];
	mutable UInt16		_bufferSpace[24];
	mutable UInt16		_stringSpace[24];
	mutable UInt16		_bufferU16Space[25];
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_rmtesbuffer_h
