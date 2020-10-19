/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_Payload_h
#define __refinitiv_ema_access_Payload_h

/**
	@class refinitiv::ema::access::Payload Payload.h "Access/Include/Payload.h"
	@brief Payload conveys the data part of item image.

	Payload contains objects of complex type.

	The following code snippet shows extracting of Payload and its content while processing UpdateMsg.

	class AppClient : public OmmConsumerClient
	{
		...

		void onUpdateMsg( const UpdateMsg& , const OmmConsumerEvent& );
	};

	void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
	{
		...

		const Payload& payload = refreshMsg.getPayload();

		switch ( payload.getDataType() )
		{
		case DataType::FieldListEnum :
			decode( payload.getFieldList() );
			break;
		case DataType::MapEnum :
			decode( payload.getMap() );
			break;
		case DataType::NoDataEnum :
			break;
		}

		...
	}

	\endcode

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform extracting of Payload and its content.
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
class OmmAnsiPage;
class OmmError;

class MsgDecoder;

class EMA_ACCESS_API Payload
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType of the contained data. 
		\remark return of DataType::NoDataEnum signifies no data present in Payload
		\remark return of DataType::OmmErrorEnum signifies error while extracting content of Payload
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
	virtual ~Payload();
	//@}

private :

	friend class Msg;

	const Data*		_pPayload;

	Payload();
	Payload( const Payload& );
	Payload& operator=( const Payload& );
};

}

}

}

#endif // __refinitiv_ema_access_Payload_h
