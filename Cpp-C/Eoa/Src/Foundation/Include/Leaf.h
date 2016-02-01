/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_leaf_h
#define __thomsonreuters_eoa_foundation_leaf_h

/**
	@class thomsonreuters::eoa::foundation::Leaf Leaf.h "Foundation/Include/Leaf.h"
	@brief Leaf class represents all OMM simple types.

	\remark All methods in this class are \ref SingleThreaded.

	@see Component,
		Tag,
		OmmReal,
		OmmDate,
		OmmTime,
		OmmDateTime,
		OmmQos,
		OmmState,
		EoaString,
		EoaBuffer,
		RmtesBuffer,
		OmmInvalidUsageException,
		OmmMemoryExhaustionException
*/

#include "Foundation/Include/Component.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class OmmReal;
class OmmDate;
class OmmTime;
class OmmDateTime;
class OmmQos;
class OmmState;
class EoaBuffer;
class RmtesBuffer;

class EOA_FOUNDATION_API Leaf : public Component
{
public:

	///@name Accessors
	//@{
	/** Returns the Int simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Int
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return Int64
	*/
	virtual Int64 getInt() const = 0;

	/** Returns the UInt simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not UInt
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return UInt64
	*/
	virtual UInt64 getUInt() const = 0;

	/** Returns the OmmReal simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Real
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmReal
	*/
	virtual const OmmReal& getReal() const = 0;

	/** Returns the float simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Float
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return Float
	*/
	virtual float getFloat() const = 0;

	/** Returns the double simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not double
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return double
	*/
	virtual double getDouble() const = 0;
	
	/** Returns the OmmDate simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Date
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmDate
	*/
	virtual const OmmDate& getDate() const = 0;
	
	/** Returns the OmmTime simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Time
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmTime
	*/
	virtual const OmmTime& getTime() const = 0;

	/** Returns the OmmDateTime simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not DateTime
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmDateTime
	*/
	virtual const OmmDateTime& getDateTime() const = 0;

	/** Returns the OmmQos simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Qos
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmQos
	*/
	virtual const OmmQos& getQos() const = 0;
	
	/** Returns the OmmState simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not State
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmState
	*/
	virtual const OmmState& getState() const = 0;

	/** Returns the enumeration simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Enum
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return UInt16
	*/
	virtual UInt16 getEnum() const = 0;

	/** Returns the Buffer simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Buffer
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return EoaBuffer
	*/
	virtual const EoaBuffer& getBuffer() const = 0;

	/** Returns the Ascii string simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Ascii
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return EoaString
	*/
	virtual const EoaString& getAscii() const = 0;
	
	/** Returns the Utf8 simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Utf8
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return EoaBuffer;
	*/
	virtual const EoaBuffer& getUtf8() const = 0;
	
	/** Returns the Rmtes simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Rmtes
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return RmtesBuffer
	*/
	virtual const RmtesBuffer& getRmtes() const = 0;
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Leaf();
	//@}

protected :

	Leaf();

private :

	Leaf( const Leaf& );
	Leaf& operator=( const Leaf& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_leaf_h
