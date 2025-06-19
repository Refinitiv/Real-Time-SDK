/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_Summary_h
#define __refinitiv_ema_access_Summary_h

/**
	@class refinitiv::ema::access::SummaryData SummaryData.h "Access/Include/SummaryData.h"
	@brief SummaryData is used to convey Omm SummaryData information optionally present on Map, Series and Vector.

	SummaryData contains objects of complex type.

	The following code snippet shows extracting of SummaryData and its content while getting data from Map.

	\code

	void decodeMap( const Map& map )
	{
		...

		const SummaryData& summaryData = map.getSummaryData();

		switch ( summaryData.getDataType() )
		{
		case DataType::FieldListEnum :
			decodeFieldList( summaryData.getFieldList() );
			break;
		case DataType::ElementListEnum :
			decodeElementList( summaryData.getElementList() );
			break;
		case DataType::NoDataEnum :
			break;
		}

		...
	}

	\endcode

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform extracting of SummaryData and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see ComplexType
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

#include "Access/Include/ComplexType.h"

namespace refinitiv {

namespace ema {

namespace access {

class ReqMsg;
class RefreshMsg;
class UpdateMsg;
class StatusMsg;
class GenericMsg;
class PostMsg;
class AckMsg;
class FieldList;
class ElementList;
class Map;
class Vector;
class Series;
class FilterList;
class OmmOpaque;
class OmmXml;
class OmmJson;
class OmmAnsiPage;
class OmmError;

class Decoder;

class EMA_ACCESS_API SummaryData
{
public:

	///@name Accessors
	//@{
	/** Returns the DataType of the contained data. 
		\remark return of DataType::NoDataEnum signifies no data present in SummaryData
		\remark return of DataType::OmmErrorEnum signifies error while extracting content of SummaryData
		@return data type of the contained object
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the complex type based on the DataType.
		@return ComplexType class reference to the contained object
	*/
	const ComplexType& getData() const;

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
		@return OmmOpaque class reference to contained object
	*/
	const OmmOpaque& getOpaque() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not OmmXml
		@return OmmXml class reference to contained object
	*/
	const OmmXml& getXml() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not OmmJson
		@return OmmJson class reference to contained object
	*/
	const OmmJson& getJson() const;

	/** Returns the current OMM data represented as a specific complex type.
		@throw OmmInvalidUsageException if contained object is not OmmAnsiPage
		@return OmmAnsiPage class reference to contained object
	*/
	const OmmAnsiPage& getAnsiPage() const;

	/** Returns Error.
		@throw OmmInvalidUsageException if contained object is not OmmError
		@return OmmError class reference to contained object
	*/
	const OmmError& getError() const;
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~SummaryData();
	//@}

private :

	friend class Map;
	friend class Series;
	friend class Vector;

	Decoder*		_pDecoder;

	SummaryData();
	SummaryData( const SummaryData& );
	SummaryData& operator=( const SummaryData& );
};

}

}

}

#endif // __refinitiv_ema_access_Summary_h
