/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
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
		EmaBuffer,
		RmtesBuffer,
		OmmInvalidUsageException,
		OmmMemoryExhaustionException
*/

#include "Foundation/Include/Component.h"
#include "Foundation/Include/EoaString.h"

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
class Tag;

class LeafDecoder;

class EOA_FOUNDATION_API Leaf : public Component
{
public:

	///@name Constructor
	//@{
	/** Constructs Leaf.
	*/
	Leaf();
	
	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Leaf();
	//@}

	///@name Accessors
	//@{
	/** Returns the component type of Leaf
		@return ComponentType
		\remark return of other than ComponentType::LeafEnum indicates lack of presence of this component
	*/
	ComponentType getComponentType() const throw() override;

	/** Returns the DataType of data contained in this leaf
		@return DataType
	*/
	DataType getDataType() const throw() override;

	/** Returns the level of depth at which this leaf is in the current nesting hierarchy
		@return the depth level
		\remark the depth level is a zero based index with the top level component assigned the 0 value
	*/
	UInt64 getDepth() const throw() override;

	/** Returns the Tag object, which identifies this leaf's key type.
		@return the Tag of this object
	*/
	const Tag& getTag() const throw() override;

	/** Returns the Int simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Int
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return Int64
	*/
	virtual Int64 getInt() const;

	/** Returns the UInt simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not UInt
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return UInt64
	*/
	virtual UInt64 getUInt() const;

	/** Returns the OmmReal simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Real
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmReal
	*/
	virtual const OmmReal& getReal() const;

	/** Returns the float simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Float
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return Float
	*/
	virtual float getFloat() const;

	/** Returns the double simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not double
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return double
	*/
	virtual double getDouble() const;
	
	/** Returns the OmmDate simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Date
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmDate
	*/
	virtual const OmmDate& getDate() const;
	
	/** Returns the OmmTime simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Time
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmTime
	*/
	virtual const OmmTime& getTime() const;

	/** Returns the OmmDateTime simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not DateTime
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmDateTime
	*/
	virtual const OmmDateTime& getDateTime() const;

	/** Returns the OmmQos simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Qos
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmQos
	*/
	virtual const OmmQos& getQos() const;
	
	/** Returns the OmmState simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not State
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return OmmState
	*/
	virtual const OmmState& getState() const;

	/** Returns the enumeration simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Enum
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return UInt16
	*/
	virtual UInt16 getEnum() const;

	/** Returns the Buffer simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Buffer
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return EoaBuffer
	*/
	virtual const EoaBuffer& getBuffer() const;

	/** Returns the Ascii string simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Ascii
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return EoaString
	*/
	virtual const EoaString& getAscii() const;
	
	/** Returns the Utf8 simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Utf8
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return EoaBuffer;
	*/
	virtual const EoaBuffer& getUtf8() const;
	
	/** Returns the Rmtes simple data type of this Leaf.
		@throw OmmInvalidUsageException if contained data is not Rmtes
		@throw OmmInvalidUsageException if contained data is not present
		@throw OmmInvalidUsageException if contained data is blank
		@return RmtesBuffer
	*/
	virtual const RmtesBuffer& getRmtes() const;

	/** Returns a string representation of this leaf.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return string representation of this leaf
	*/
	const EoaString& toString() const override;

	/** Returns a string representation of this leaf's value.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return string representation of this leaf's value
	*/
	const EoaString& getValue() const override;

	/** checks if this Component is locally cached.
		@return true if this leaf is locally cached; false otherwise
	*/
	bool isLocal() const throw() override;

	/** checks if this leaf is non blank.
		@return true if this leaf is non blank; false otherwise
	*/
	bool isNonBlank() const throw() override;

	/** checks if this leaf is present.
		@return true if this leaf is present; false otherwise
	*/
	bool isPresent() const throw() override;
	//@}

private :

	friend class NoLeaf;

	mutable EoaString	_toString;
	LeafDecoder*		_pDecoder;

	Decoder& getDecoder() override;

	const EoaString& toString( UInt64 indent, bool ) const override;

	Leaf( const Leaf& );
	Leaf& operator=( const Leaf& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_leaf_h
