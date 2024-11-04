/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019,2024 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_VectorEntry_h
#define __refinitiv_ema_access_VectorEntry_h

/**
	@class refinitiv::ema::access::VectorEntry VectorEntry.h "Access/Include/VectorEntry.h"
	@brief VectorEntry represents an entry of Vector.
	
	VectorEntry associates entry's position, action, permission information, data and its data type.
	
	\code

	decodeVector( const Vector& vector )
	{
		while ( vector.forth() )
		{
			const VectorEntry& vectorEntry = vector.getEntry();

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
		OmmJson,
		OmmAnsiPage,
		OmmError
*/
 
#include "Access/Include/OmmError.h"

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
class ElementList;
class Map;
class Series;
class FilterList;
class OmmOpaque;
class OmmXml;
class OmmJson;
class OmmAnsiPage;
class Vector;
class OmmError;

class VectorDecoder;

class EMA_ACCESS_API VectorEntry
{
public :

	/** @enum VectorAction
		An enumeration representing vector entry action.
	*/
	enum VectorAction
	{
		UpdateEnum	= 1,		/*!< Indicates a partial change of the entry */

		SetEnum		= 2,		/*!< Indicates to replace the entry. */

		ClearEnum	= 3,		/*!< Indicates to empty the entry. Contains no data. */

		InsertEnum	= 4,		/*!< Indicates to place the entry in between other entries.
									Increases any higher-ordered position by one. May leave gaps
									if previous lower-ordered position is unpopulated. 
									Only valid if sortable() returns true. */

		DeleteEnum	= 5			/*!< Indicates to remove the entry. Decreases any higher-ordered
									position by one. Only valid if sortable() returns true.
									Contains no data.  */
	};

	///@name Accessors
	//@{
	/** Returns the VectorAction value as a string format.
		@return EmaString containing string representation of VectorAction
	*/
	const EmaString& getVectorActionAsString() const;
		
	/** Returns the DataType of the entry's load. 
		\remark return of DataType::NoDataEnum signifies no data present in load
		\remark return of DataType::OmmErrorEnum signifies error while extracting content of load
		@return data type of the contained object
	*/
	DataType::DataTypeEnum getLoadType() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Indicates presence of PermissionData.
		@return true if permission data is set; false otherwise
	*/
	bool hasPermissionData() const;

	/** Returns position of the entry.
		@return entry's position
	*/
	UInt32 getPosition() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not ReqMsg
		@return ReqMsg class reference to contained object
	*/
	const ReqMsg& getReqMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not RefreshMsg
		@return RefreshMsg class reference to contained entry's load object
	*/
	const RefreshMsg& getRefreshMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not UpdateMsg
		@return UpdateMsg class reference to contained entry's load object
	*/
	const UpdateMsg& getUpdateMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not StatusMsg
		@return StatusMsg class reference to contained entry's load object
	*/
	const StatusMsg& getStatusMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not PostMsg
		@return PostMsg class reference to contained entry's load object
	*/
	const PostMsg& getPostMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not AckMsg
		@return AckMsg class reference to contained entry's load object
	*/
	const AckMsg& getAckMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not GenericMsg
		@return GenericMsg class reference to contained entry's load object
	*/
	const GenericMsg& getGenericMsg() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not FieldList
		@return FieldList class reference to contained entry's load object
	*/
	const FieldList& getFieldList() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not ElementList
		@return ElementList class reference to contained entry's load object
	*/
	const ElementList& getElementList() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not Map
		@return Map class reference to contained entry's load object
	*/
	const Map& getMap() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not Vector
		@return Vector class reference to contained entry's load object
	*/
	const Vector& getVector() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not Series
		@return Series class reference to contained entry's load object
	*/
	const Series& getSeries() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not FilterList
		@return FilterList class reference to contained entry's load object
	*/
	const FilterList& getFilterList() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not OmmOpaque
		@return OmmOpaque class reference to contained entry's load object
	*/
	const OmmOpaque& getOpaque() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not OmmXml
		@return OmmXml class reference to contained entry's load object
	*/
	const OmmXml& getXml() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not OmmJson
		@return OmmJson class reference to contained entry's load object
	*/
	const OmmJson& getJson() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not OmmAnsiPage
		@return OmmAnsiPage class reference to contained entry's load object
	*/
	const OmmAnsiPage& getAnsiPage() const;

	/** Returns the contained Data based on the DataType.
		@return Data class reference to contained entry's load object
	*/
	const Data& getLoad() const;

	/** Returns the current action on the OMM data.
		@return VectorEntry Action
	*/
	VectorAction getAction() const;

	/** Returns PermissionData.
		@throw OmmInvalidUsageException if hasPermissionData() returns false
		@return EmaBuffer containing permission information
	*/
	const EmaBuffer& getPermissionData() const;

	/** Returns Error.
		@throw OmmInvalidUsageException if contained object is not OmmError
		@return OmmError class reference to contained entry's load object
	*/
	const OmmError& getError() const;
	//@}

private :

	friend class Vector;
	
	VectorDecoder*			_pDecoder;
	const Data*				_pLoad;

	mutable EmaString		_toString;

	VectorEntry();
	virtual ~VectorEntry();

	VectorEntry( const VectorEntry& );
	VectorEntry& operator=( const VectorEntry& );
};

} 

}

}

#endif // __refinitiv_ema_access_VectorEntry_h
