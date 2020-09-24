/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_SeriesEntry_h
#define __rtsdk_ema_access_SeriesEntry_h

/**
	@class rtsdk::ema::access::SeriesEntry SeriesEntry.h "Access/Include/SeriesEntry.h"
	@brief SeriesEntry represents an entry of Series.

	SeriesEntry associates entry's data and its data type.
	
	\code

	decodeSeries( const Series& series )
	{
		while ( series.forth() )
		{
			const SeriesEntry& seriesEntry = series.getEntry();

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
		OmmError
*/

#include "Access/Include/Data.h"

namespace rtsdk {

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
class Vector;
class FilterList;
class OmmOpaque;
class OmmXml;
class OmmAnsiPage;
class OmmError;
class Series;

class SeriesDecoder;

class EMA_ACCESS_API SeriesEntry
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType of the entry's load. 
		\remark return of DataType::OmmErrorEnum signifies error while extracting content of load
		@return data type of the contained object
	*/
	DataType::DataTypeEnum getLoadType() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

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

	/** Returns Error.
		@throw OmmInvalidUsageException if contained object is not OmmError
		@return OmmError class reference to contained entry's load object
	*/
	const OmmError& getError() const;
	//@}

private :

	friend class Series;
	
	SeriesDecoder*			_pDecoder;
	const Data*				_pLoad;

	mutable EmaString		_toString;

	SeriesEntry();
	virtual ~SeriesEntry();
		
	SeriesEntry( const SeriesEntry& );
	SeriesEntry& operator=( const SeriesEntry& );
};

}

}

}

#endif //__rtsdk_ema_access_SeriesEntry_h
