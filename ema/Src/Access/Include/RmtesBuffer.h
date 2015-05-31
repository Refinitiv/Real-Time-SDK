/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_RmtesBuffer_h
#define __thomsonreuters_ema_access_RmtesBuffer_h

/**
	@class thomsonreuters::ema::access::RmtesBuffer RmtesBuffer.h "Access/Include/Include/RmtesBuffer.h"
	@brief RmtesBuffer represents RMTES data.

	RmtesBuffer stores and applies RMTES data.

	\remark RmtesBuffer class contains a copy of the buffer passed on apply methods.
	\remark All methods in this class are \ref SingleThreaded.
*/

#include "Access/Include/Common.h"

namespace thomsonreuters{

namespace ema {

namespace access {

class EmaBuffer;
class EmaString;
class EmaBufferU16;
class RmtesBufferImpl;

class OmmRmtesDecoder;

class EMA_ACCESS_API RmtesBuffer
{
public :

	///@name Constructor
	//@{
	/** Constructs RmtesBuffer.
	*/
	RmtesBuffer();

	/** Constructs RmtesBuffer.
		\remark Preallocates memory if length is different than 0
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] length specifies size of preallocated memory
	*/
	RmtesBuffer( UInt32 length );

	/** Assignment constructor
		@throw OmmMemoryExhaustionException if app runs out of memory
		@throw OmmInvalidUsageException if fails to apply passed in content
		@param[in] buf pointer to the memory containing copied in buffer
		@param[in] length specifies number of characters to copy
	*/
	RmtesBuffer( const char* buf, UInt32 length );

	/** Copy constructor.
		@throw OmmMemoryExhaustionException if app runs out of memory
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
		@throw OmmMemoryExhaustionException if app runs out of memory
		@throw OmmInvalidUsageException if fails to convert
		@return EmaBuffer containing RMTES data converted to UTF8
	*/
	const EmaBuffer& getAsUTF8() const;

	/** Returns the content converted as UTF16.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@throw OmmInvalidUsageException if fails to convert
		@return EmaBufferU16 containing RMTES data converted to UTF16
	*/
	const EmaBufferU16& getAsUTF16() const;

	/** Returns a string representation of the class instance which is converted to UTF8.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@throw OmmInvalidUsageException if fails to convert
		@return EmaString containing RMTES data converted to UTF8
	*/
	const EmaString& toString() const;
	//@}

	///@name Operations
	//@{
	/** Clears contained content.
		@return reference to this object
	*/
	RmtesBuffer& clear();

	/** apply passed in RMTES data
		@throw OmmMemoryExhaustionException if app runs out of memory
		@throw OmmInvalidUsageException if fails to apply
		@param[in] buf specifies RmtesBuffer to be applied to this object
		@return reference to this object
	*/
	RmtesBuffer& apply( const RmtesBuffer& buf );

	/** apply passed in RMTES data
		@throw OmmMemoryExhaustionException if app runs out of memory
		@throw OmmInvalidUsageException if fails to apply
		@param[in] buf specifies pointer to the memory containing RMTES data to be applied to this object
		@param[in] length specifies size of the memory to be applied to this object
		@return reference to this object
	*/
	RmtesBuffer& apply( const char* buf, UInt32 length );

	/** apply passed in RMTES data
		@throw OmmMemoryExhaustionException if app runs out of memory
		@throw OmmInvalidUsageException if fails to apply
		@param[in] buf specifies EmaBuffer containing RMTES string to be applied to this object
		@return reference to this object
	*/
	RmtesBuffer& apply( const EmaBuffer& buf );
	//@}

private :

	friend class OmmRmtesDecoder;

	RmtesBufferImpl*	_pImpl;
	UInt64				_space[22];
};

}

}

}

#endif // __thomsonreuters_ema_access_RmtesBuffer_h
