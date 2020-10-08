/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ElementEntry_h
#define __refinitiv_ema_access_ElementEntry_h

/**
	@class refinitiv::ema::access::ElementEntry ElementEntry.h "Access/Include/ElementEntry.h"
	@brief ElementEntry represents an entry of ElementList.

	ElementEntry associates entry's name, data and its data type.

	\code

	decodeElementList( const ElementList& eList )
	{
		while ( elist.forth() )
		{
			const ElementEntry& elementEntry = elist.getEntry();

			...
		}
	}

	\endcode

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform extracting of data from entry.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		ComplexType
		ReqMsg,
		RefreshMsg,
		UpdateMsg,
		StatusMsg,
		GenericMsg,
		PostMsg,
		AckMsg,
		FieldList,
		ElementList,
		Map,
		Vector,
		Series,
		FilterList,
		OmmOpaque,
		OmmXml,
		OmmAnsiPage,
		OmmError,
		EmaBuffer,
		EmaString
*/

#include "Access/Include/Data.h"

namespace refinitiv {
	
namespace ema {

namespace access {

class ReqMsg;
class RefreshMsg;
class StatusMsg;
class UpdateMsg;
class GenericMsg;
class PostMsg;
class AckMsg;
class FieldList;
class Map;
class Vector;
class Series;
class FilterList;
class OmmArray;
class OmmOpaque;
class OmmXml;
class OmmAnsiPage;
class OmmReal;
class OmmDate;
class OmmTime;
class OmmDateTime;
class OmmQos;
class OmmState;
class ElementList;
class OmmError;
class RmtesBuffer;

class ElementListDecoder;

class EMA_ACCESS_API ElementEntry
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType of the entry's load.
		\remark return of DataType::NoDataEnum signifies no data present in load
		\remark return of DataType::OmmErrorEnum signifies error while extracting content of load
		@return data type of the contained object
	*/
	DataType::DataTypeEnum getLoadType() const;

	/** Returns the Code of the entry's load. The code indicates a special state of a Data.
		\remark Attempts to extract data will cause OmmInvalidUsageException if Data::BlankEnum is returned.
		@return data code of the contained object 
	*/
	Data::DataCode getCode() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Returns name of the entry. 
		@return EmaString containing name of the entry
	*/
	const EmaString& getName() const;

	/** Returns the current OMM Data.
		@return Data class reference to contained object
	*/
	const Data& getLoad() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not ReqMsg
		@return ReqMsg class reference to contained object
	*/
	const ReqMsg& getReqMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not RefreshMsg
		@return RefreshMsg class reference to contained object
	*/
	const RefreshMsg& getRefreshMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not UpdateMsg
		@return UpdateMsg class reference to contained object
	*/
	const UpdateMsg& getUpdateMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not StatusMsg
		@return StatusMsg class reference to contained object
	*/
	const StatusMsg& getStatusMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not PostMsg
		@return PostMsg class reference to contained object
	*/
	const PostMsg& getPostMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not AckMsg
		@return AckMsg class reference to contained object
	*/
	const AckMsg& getAckMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not GenericMsg
		@return GenericMsg class reference to contained object
	*/
	const GenericMsg& getGenericMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not FieldList
		@return FieldList class reference to contained object
	*/
	const FieldList& getFieldList() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not ElementList
		@return ElementList class reference to contained object
	*/
	const ElementList& getElementList() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not Map
		@return Map class reference to contained object
	*/
	const Map& getMap() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not Vector
		@return Vector class reference to contained object
	*/
	const Vector& getVector() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not Series
		@return Series class reference to contained object
	*/
	const Series& getSeries() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not FilterList
		@return FilterList class reference to contained object
	*/
	const FilterList& getFilterList() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not OmmOpaque
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmOpaque class reference to contained object
	*/
	const OmmOpaque& getOpaque() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not OmmXml
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmXml class reference to contained object
	*/
	const OmmXml& getXml() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not OmmAnsiPage
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmAnsiPage class reference to contained object
	*/
	const OmmAnsiPage& getAnsiPage() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmInt
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return Int64
	*/
	Int64 getInt() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmUInt
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return UInt64
	*/
	UInt64 getUInt() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmReal
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmReal class reference to contained object
	*/
	const OmmReal& getReal() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmFloat
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return float
	*/
	float getFloat() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmDouble
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return double
	*/
	double getDouble() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmData
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmDate class reference to contained object
	*/
	const OmmDate& getDate() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmTime
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return ommTime class reference to contained object
	*/
	const OmmTime& getTime() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmDateTime
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmDateTime class reference to contained object
	*/
	const OmmDateTime& getDateTime() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmQos
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmQos class reference to contained object
	*/
	const OmmQos& getQos() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmState
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmState class reference to contained object
	*/
	const OmmState& getState() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmEnum
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return UInt16
	*/
	UInt16 getEnum() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmBuffer
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return EmaBuffer
	*/
	const EmaBuffer& getBuffer() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmAscii
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return EmaString
	*/
	const EmaString& getAscii() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmUtf8
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return EmaBuffer
	*/
	const EmaBuffer& getUtf8() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmRmtes
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return RmtesBuffer
	*/
	const RmtesBuffer& getRmtes() const;

	/** Returns current OMM data represented as an OmmArray.
		@throw OmmInvalidUsageException if contained object is not OmmArray
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmArray class reference to contained object
	*/
	const OmmArray& getArray() const;

	/** Returns Error.
		@throw OmmInvalidUsageException if contained object is not OmmError
		@return OmmError class reference to contained object
	*/
	const OmmError& getError() const;
	//@}

private :
	
	friend class ElementList;

	ElementListDecoder*		_pDecoder;
	const Data*	const*		_pLoad;

	mutable EmaString		_toString;

	ElementEntry();
	virtual ~ElementEntry();

	ElementEntry( const ElementEntry& );
	ElementEntry& operator=( const ElementEntry& );
};

}

}

}

#endif // __refinitiv_ema_access_ElementEntry_h
