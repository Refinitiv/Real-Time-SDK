/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_tag_h
#define __thomsonreuters_eoa_foundation_tag_h

/**
	@class thomsonreuters::eoa::foundation::Tag Tag.h "Foundation/Include/Tag.h"
	@brief Tag class represents Component name or id.

	\remark All methods in this class are \ref SingleThreaded.

	@see Component,
		TagType,
		EoaString,
		EoaBuffer,
		RmtesBuffer,
		OmmReal,
		OmmDate,
		OmmDateTime,
		OmmQos,
		OmmState,
		OmmTime,
		OmmInvalidUsageException,
		OmmMemoryExhaustionException
*/

#include "Foundation/Include/EoaString.h"
#include "Foundation/Include/TagType.h"
#include "Foundation/Include/OmmDate.h"
#include "Foundation/Include/OmmDateTime.h"
#include "Foundation/Include/OmmQos.h"
#include "Foundation/Include/OmmReal.h"
#include "Foundation/Include/OmmState.h"
#include "Foundation/Include/OmmTime.h"
#include "Foundation/Include/RmtesBuffer.h"

namespace thomsonreuters {
	
namespace eoa {
	
namespace foundation {

class EoaBuffer;

class EOA_FOUNDATION_API Tag
{
public:
	
	///@name Accessors
	//@{
	/** Returns the type of this Tag
		@return TagType
	*/
	TagType getTagType() const throw();

	/** Returns the tag value of Array
		@throw OmmInvalidUsageException if this object has different TagType
		@return UInt16
	*/
	UInt16 getTagArray() const;

	/** Returns the tag value of ElementList
		@throw OmmInvalidUsageException if this object has different TagType
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return EoaString
	*/
	const EoaString& getTagElementList() const;

	/** Returns the tag value of FieldList by Field ID
		@throw OmmInvalidUsageException if this object has different TagType
		@return Int16
	*/
	Int16 getTagFieldList() const;

	/** Returns the tag value of FieldList by Field name
		@throw OmmInvalidUsageException if this object has different TagType
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return EoaString
	*/
	const EoaString& getTagFieldListName() const;

	/** Returns the tag value of FilterList
		@throw OmmInvalidUsageException if this object has different TagType
		@return UInt8
	*/
	UInt8 getTagFilterList() const;

	/** Returns the tag value of Map with Ascii key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return EoaString
	*/
	const EoaString& getTagMapAscii() const;

	/** Returns the tag value of Map with Buffer key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return EoaBuffer
	*/
	const EoaBuffer& getTagMapBuffer() const;

	/** Returns the tag value of Map with Date key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return OmmDate
	*/
	const OmmDate& getTagMapDate() const;

	/** Returns the tag value of Map with DateTime key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return EoaOmmDateTime
	*/
	const OmmDateTime& getTagMapDateTime() const;

	/** Returns the tag value of Map with double key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return double
	*/
	double getTagMapDouble() const;

	/** Returns the tag value of Map with enumeration key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return UInt16
	*/
	UInt16 getTagMapEnum() const;

	/** Returns the tag value of Map with Float key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return Float
	*/
	float getTagMapFloat() const;

	/** Returns the tag value of Map with Int key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return Int64
	*/
	Int64 getTagMapInt() const;

	/** Returns the tag value of Map with Qos key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return OmmQos
	*/
	const OmmQos& getTagMapQos() const;
	
	/** Returns the tag value of Map with Real key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return OmmReal
	*/
	const OmmReal& getTagMapReal() const;

	/** Returns the tag value of Map with Rmtes key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return RmtesBuffer
	*/
	const RmtesBuffer& getTagMapRmtes() const;

	/** Returns the tag value of Map with State key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return OmmState
	*/
	const OmmState& getTagMapState() const;
	
	/** Returns the tag value of Map with Time key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return OmmTime
	*/
	const OmmTime& getTagMapTime() const;

	/** Returns the tag value of Map with UInt key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return UInt64
	*/
	UInt64 getTagMapUInt() const;

	/** Returns the tag value of Map with Utf8 key data type
		@throw OmmInvalidUsageException if this object has different TagType
		@return EoaBuffer
	*/
	const EoaBuffer& getTagMapUtf8() const;

	/** Returns the tag value of Series
		@throw OmmInvalidUsageException if this object has different TagType
		@return UInt16
	*/
	UInt16 getTagSeries() const;

	/** Returns the tag value of Vector
		@throw OmmInvalidUsageException if this object has different TagType
		@return UInt32
	*/
	UInt32 getTagVector() const;

	/** Returns a string representation of this object (e.g. tag type and tag's hex value).
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return string representation of this object
	*/
	const EoaString& toString() const;

	/** Returns a hexadecimal string representation of the tag's value.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return hexadecimal string representation of the tag's value
	*/
	const EoaString& getHexValue() const;

	/** Returns a string representation of the tag's value.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return string representation of the tag's value
	*/
	const EoaString& getValue() const;

	/** Operator const char* overload.
		@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	operator const char* () const;
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Tag();
	//@}

private:

	friend class Leaf;
	friend class Node;
	friend class NoComponent;
	friend class NoLeaf;
	friend class NoNode;

	friend class Decoder;
	friend class LeafDecoder;
	friend class NodeDecoder;
	friend class CacheEntry;
	friend class CacheNode;
	friend class CacheLeaf;

	TagType						_tagType;
	mutable EoaString			_toString;
	mutable EoaString			_valueString;
	mutable EoaString			_hexValueString;
	UInt16						_position;
	const void*					_pEntry;
	mutable OmmDate				_ommDate;
	mutable OmmDateTime			_ommDateTime;
	mutable OmmReal				_ommReal;
	mutable OmmState			_ommState;
	mutable OmmQos				_ommQos;
	mutable OmmTime				_ommTime;
	mutable UInt16				_eoaBuffer[24];
	mutable RmtesBuffer			_rmtesBuffer;
	void*						_pCacheEntry;

	const EoaString& toString( UInt64 indent, bool ) const;

	Tag();
	Tag( const Tag& );
	Tag& operator=( const Tag& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_tag_h
