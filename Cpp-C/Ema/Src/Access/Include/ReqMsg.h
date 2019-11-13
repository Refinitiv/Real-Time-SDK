/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ReqMsg_h
#define __thomsonreuters_ema_access_ReqMsg_h

/**
	@class thomsonreuters::ema::access::ReqMsg ReqMsg.h "Access/Include/ReqMsg.h"
	@brief ReqMsg allows consumer application to express its interest in an item.

	Among other attributes, ReqMsg conveys item's name, service, domain type, and
	desired quality of service. ReqMsg may also be used to: specify application interest
	in a dynamic view, open a batch of items, or request a symbol list item with its data.

	The following code snippet shows specification of ReqMsg and registration of interest
	in this item.

	\code

	class AppClient : public OmmConsumerClient { ... }

	AppClient client;
	OmmConsumer consumer( OmmConsumerConfig().host( "1.1.1.1:14002" ) );
	ReqMsg reqMsg;
	reqMsg.name( "IBM.N" ).serviceName( "DIRECT_FEED" ).priority( 2, 1 );

	UInt64 itemHandle = consumer.registerClient( reqMsg, appClient );

	\endcode

	\remark Calling get***() method on an optional member of ReqMsg must be preceded by a
			call to respective has***() method.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and getting of information from ReqMsg.
	\remark Objects of this class are not cache-able.
	\remark	Decoding of just encoded ReqMsg in the same application is not supported.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		Msg,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Msg.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API ReqMsg : public Msg
{
public :

	/** @enum Rate
		An enumeration representing Qos Rate.
	*/
	enum Rate
	{
		TickByTickEnum = 0,						/*!< Rate is Tick By Tick, indicates every change to
													information is conveyed */

		JustInTimeConflatedEnum = 0xFFFFFF00,	/*!< Rate is Just In Time Conflated, indicates extreme
													bursts of data may be conflated */

		BestConflatedRateEnum = 0xFFFFFFFF,		/*!< Request Rate with range from 1 millisecond conflation to maximum conflation. */

		BestRateEnum = 0xFFFFFFFE				/*!< Request Rate with range from tick-by-tick to maximum conflation. */
	};

	/** @enum Timeliness
		An enumeration representing Qos Timeliness.
	*/
	enum Timeliness
	{
		RealTimeEnum = 0,							/*!< Timeliness is RealTime, indicates information is updated
														as soon as new information becomes available */

		BestDelayedTimelinessEnum = 0xFFFFFFFF,		/*!< Request Timeliness with range from one second delay to maximum delay. */

		BestTimelinessEnum = 0xFFFFFFFE				/*!< Request Timeliness with range from real-time to maximum delay. */
	};

	///@name Constructor
	//@{
	/** Constructs ReqMsg.
	 */
	ReqMsg();

	/** Copy constructor.
		\remark this is used to copy and process ReqMsg outside of EMA's callback methods.
		\remark this method does not support passing in just encoded ReqMsg in the application space.
	*/
	ReqMsg( const ReqMsg& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	 */
	virtual ~ReqMsg();
	//@}

	///@name Accessors
	//@{
	/** Returns the Rate value as a string format.
		@return string representation of Qos Rate
	*/
	const EmaString& getRateAsString() const;

	/** Returns the Timeliness value as a string format.
		@return string representation of Qos Timeliness
	*/
	const EmaString& getTimelinessAsString() const;

	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::ReqMsgEnum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::NoCodeEnum
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return a buffer with the message hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Indicates presence of Priority.
		\remark Priority is an optional member of ReqMsg
		@return true if priority is set; false otherwise
	*/
	bool hasPriority() const;

	/** Indicates presence of Qos.
		\remark Qos is an optional member of ReqMsg
		@return true if Qos is set; false otherwise
	*/
	bool hasQos() const;

	/** Indicates presence of View.
		\remark View specification is an optional member of ReqMsg
		@return true if View is set; false otherwise
	*/
	bool hasView() const;

	/** Indicates presence of Batch.
		\remark Batch specification is an optional member of ReqMsg
		@return true if Batch specification is set; false otherwise
	*/
	bool hasBatch() const;

	/** Indicates presence of the ServiceName within the MsgKey.
		\remark ServiceName is an optional member of ReqMsg.
		@return true if service name is set; false otherwise
	*/
	bool hasServiceName() const;

	/** Returns PriorityClass.
		@throw OmmInvalidUsageException if hasPriority() returns false
		@return priority class
	*/
	UInt8 getPriorityClass() const;

	/** Returns PriorityCount.
		@throw OmmInvalidUsageException if hasPriority() returns false
		@return priority count
	*/
	UInt16 getPriorityCount() const;

	/** Returns QosTimeliness.
		@throw OmmInvalidUsageException if hasQos() returns false
		@return Qos Timeliness
	*/
	UInt32 getQosTimeliness() const;

	/** Returns QosRate.
		@throw OmmInvalidUsageException if hasQos() returns false
		@return Qos Rate
	*/
	UInt32 getQosRate() const;

	/** Returns InitialImage.
		@return true if an initial image is requested; false otherwise
	*/
	bool getInitialImage() const;

	/** Returns InterestAfterRefresh.
		@return true if an interest after refresh is requested; false otherwise
	*/
	bool getInterestAfterRefresh() const;

	/** Returns ConflatedInUpdates.
		@return true if conflation is requested; false otherwise
	*/
	bool getConflatedInUpdates() const;

	/** Returns Pause.
		@return true if pause is requested; false otherwise
	*/
	bool getPause() const;

	/** Returns PrivateStream.
		@return true if private stream is requested; false otherwise
	*/
	bool getPrivateStream() const;

	/** Returns the ServiceName within the MsgKey.
		@throw OmmInvalidUsageException if hasServiceName() returns false
		@return service name
	*/
	const EmaString& getServiceName() const;
	//@}

	///@name Operations
	//@{
	/** Clears the ReqMsg.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	ReqMsg& clear();

	/** Specifies StreamId.
		@param[in] streamId specifies stream id
		@return reference to this object
	*/
	ReqMsg& streamId( Int32 streamId );

	/** Specifies DomainType.
		@throw OmmUnsupportedDomainTypeException if domainType is greater than 255
		@param[in] domainType specifies RDM Message Model Type (default value is rdm::MMT_MARKET_PRICE)
		@return reference to this object
	*/
	ReqMsg& domainType( UInt16 domainType = rdm::MMT_MARKET_PRICE );

	/** Specifies Name.
		@param[in] name specifies item name
		@return reference to this object
	*/
	ReqMsg& name( const EmaString& name );

	/** Specifies NameType.
		@param[in] nameType specifies RDM Instrument NameType (default value is rdm::INSTRUMENT_NAME_RIC)
		@return reference to this object
	*/
	ReqMsg& nameType( UInt8 nameType = rdm::INSTRUMENT_NAME_RIC );

	/** Specifies ServiceName.
		\remark One service identification must be set, either id or name.
		@throw OmmInvalidUsageException if service id is already set
		@param[in] serviceName specifies service name
		@return reference to this object
	*/
	ReqMsg& serviceName( const EmaString& serviceName );

	/** Specifies ServiceId.
		\remark One service identification must be set, either id or name.
		@throw OmmInvalidUsageException if service name is already set
		@param[in] serviceId specifies service id
		@return reference to this object
	*/
	ReqMsg& serviceId( UInt32 serviceId );

	/** Specifies Id.
		@param[in] id specifies Id
		@return reference to this object
	*/
	ReqMsg& id( Int32 id );

	/** Specifies Filter.
		@param[in] filter specifies filter
		@return reference to this object
	*/
	ReqMsg& filter( UInt32 filter );

	/** Specifies Priority.
		@param[in] priorityClass specifies priority class
		@param[in] priorityCount specifies priority count within priority class
		@return reference to this object
	*/
	ReqMsg& priority( UInt8 priorityClass = 1, UInt16 priorityCount = 1 );

	/** Specifies Qos as timeliness and rate.
		@param[in] timeliness specifies Qos Timeliness (default value is BestTimelinessEnum)
		@param[in] rate specifies Qos rate (default value is BestRateEnum)
		@return reference to this object
	*/
	ReqMsg& qos( UInt32 timeliness = BestTimelinessEnum, UInt32 rate = BestRateEnum );

	/** Specifies Attrib.
		@param[in] attrib an object of ComplexType
		@return reference to this object
	*/
	ReqMsg& attrib( const ComplexType& data );

	/** Specifies Payload.
		@param[in] payload an object of ComplexType
		@return reference to this object
	*/
	ReqMsg& payload( const ComplexType& data );

	/** Specifies ExtendedHeader.
		@param[in] buffer an EmaBuffer containing extendedHeader information
		@return reference to this object
	*/
	ReqMsg& extendedHeader( const EmaBuffer& Buffer );

	/** Specifies InitialImage.
		@param[in] initialImage specifies if initial image / refresh is requested (default value is true)
		@return reference to this object
	*/
	ReqMsg& initialImage( bool initialImage = true );

	/** Specifies InterestAfterRefresh.
		@param[in] interestAfterRefresh specifies if streaming or snapshot item is requested default value is true / streaming item)
		@return reference to this object
	*/
	ReqMsg& interestAfterRefresh( bool interestAfterRefresh = true );

	/** Specifies Pause.
		@param[in] pause specifies if pause is requested (default value is false)
		@return reference to this object
	*/
	ReqMsg& pause( bool pause = false );

	/** Specifies ConflatedInUpdates.
		@param[in] conflatedInUpdates specifies if conflated update is requested (default value is false)
		@return reference to this object
	*/
	ReqMsg& conflatedInUpdates( bool conflatedInUpdates = false );

	/** Specifies PrivateStream.
		@param[in] privateStream specifies if private stream is requested (default value is false)
		@return reference to this object
	*/
	ReqMsg& privateStream( bool privateStream = false );
	//@}

private :

	friend class MarketItemHandler;
	friend class DirectoryHandler;
	friend class DictionaryHandler;

	const EmaString& toString( UInt64 ) const;

	Decoder& getDecoder();

	ReqMsg& operator=( const ReqMsg& );

	mutable EmaString		_toString;
};

}

}

}

#endif // __thomsonreuters_ema_access_ReqMsg_h
