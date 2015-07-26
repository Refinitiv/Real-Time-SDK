/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_MapEntry_h
#define __thomsonreuters_ema_access_MapEntry_h

/**
	@class thomsonreuters::ema::access::MapEntry MapEntry.h "Access/Include/MapEntry.h"
	@brief MapEntry represents an entry of Map.

	MapEntry associates entry's key, permission information, action, data and its data type.

	\code

	decodeMap( const Map& map )
	{
		while ( !map.forth() )
		{
			const MapEntry& mapEntry = map.getEntry();

			if ( mapEntry.hasPermissionData() )
			{
				const EmaBuffer& permissionData = mapEntry.getPermissionData();

				...
			}

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

#include "Access/Include/Key.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class ReqMsg;
class RefreshMsg;
class StatusMsg;
class UpdateMsg;
class GenericMsg;
class PostMsg;
class AckMsg;
class ElementList;
class FieldList;
class Map;
class Vector;
class Series;
class FilterList;
class OmmOpaque;
class OmmXml;
class OmmAnsiPage;
class OmmError;

class MapDecoder;

class EMA_ACCESS_API MapEntry
{
public :

	/** @enum MapAction
		An enumeration representing map entry action.
	*/
	enum MapAction
	{
		UpdateEnum	= 1,	/*!< Indicates a partial change of the current Omm data. */

		AddEnum		= 2,	/*!< Indicates to append or replace the current Omm data. */

		DeleteEnum	= 3		/*!< Indicates to remove current Omm data. */
	};

	///@name Accessors
	//@{
	/** Returns the MapAction value as a string format.
		@return EmaString containing string representation of MapAction
	*/
	const EmaString& getMapActionAsString() const;
		
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
		@throw OmmInvalidUsageException if contained object is not OmmAnsiPage
		@return OmmAnsiPage class reference to contained entry's load object
	*/
	const OmmAnsiPage& getAnsiPage() const;

	/** Returns the contained Data based on the DataType.
		@return Data class reference to contained entry's load object
	*/
	const Data& getLoad() const;

	/** Returns the contained key Data based on the key DataType.
		@return Key class reference to contained entry's Key object
	*/
	const Key& getKey() const;

	/** Returns the current action on the OMM data.
		@return MapAction
	*/
	MapAction getAction() const;

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

	friend class Map;
	
	MapDecoder*				_pDecoder;
	const Data*				_pLoad;
	Key						_key;

	mutable EmaString		_toString;

	MapEntry();
	virtual ~MapEntry();

	MapEntry( const MapEntry& );
	MapEntry& operator=( const MapEntry& );
};

}

}

}

#endif // __thomsonreuters_ema_access_MapEntry_h
