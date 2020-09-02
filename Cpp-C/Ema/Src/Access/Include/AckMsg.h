/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_AckMsg_h
#define __thomsonreuters_ema_access_AckMsg_h

/**
	@class rtsdk::ema::access::AckMsg AckMsg.h "Access/Include/AckMsg.h"
	@brief AckMsg indicates success or failure of PostMsg.

	If requested, AckMsg is sent by provider acting on PostMsg received from consumer.
	AckMsg indicates success or failure of caching / processing of received PostMsg.
	AckMsg is optional. Consumer requests provider to send AckMsg by calling PostMsg::solicitAck( true ).

	The following code snippet shows processing of received AckMsg.

	\code

	class AppClient : public OmmConsumerClient
	{
		...

		void onAckMsg( const AckMsg& , const OmmConsumerEvent& );
	};

	void AppClient::onAckMsg( const AckMsg& ackMsg, const OmmConsumerEvent& )
	{
		if ( ackMsg.hasName() )
			cout << ackMsg.getName() << "\n";

		if ( ackMsg.hasServiceName() )
			cout << ackMsg.getServiceName() << "\n";

		if ( ackMsg.hasNackCode() )
			cout << ackMsg.getNackCodeAsString() << "\n";
	}

	\endcode

	\remark Calling get***() method on an optional member of AcMsg must be preceded by a
			call to respective has***() method.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and getting of information from AckMsg.
	\remark Objects of this class are not cache-able.
	\remark	Decoding of just encoded AckMsg in the same application is not supported.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		Msg,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Msg.h"

namespace rtsdk {

namespace ema {

namespace access {

class EMA_ACCESS_API AckMsg : public Msg
{
public :

	/** @enum NackCode
		An enumeration representing negative acknowledgement code.
	*/
	enum NackCode
	{
		NoneEnum			= 0,	/*!< Indicates no nack code */

		AccessDeniedEnum	= 1,	/*!< Indicates access denied */

		DeniedBySourceEnum	= 2,	/*!< Indicates denied by source */

		SourceDownEnum		= 3,	/*!< Indicates source is down */

		SourceUnknownEnum	= 4,	/*!< Indicates source is unknown */

		NoResourcesEnum		= 5,	/*!< Indicates no resources */

		NoResponseEnum		= 6,	/*!< Indicates no response */

		GatewayDownEnum		= 7,	/*!< Indicates gateway down */

		SymbolUnknownEnum	= 10,	/*!< Indicates unknown symbol */

		NotOpenEnum			= 11,	/*!< Indicates not open */

		InvalidContentEnum	= 12	/*!< Indicates invalid content */
	};

	///@name Constructor
	//@{
	/** Constructs AckMsg.
	*/
	AckMsg();

	/** Copy constructor.
		\remark this is used to copy and process AckMsg outside of EMA's callback methods.
		\remark this method does not support passing in just encoded AckMsg in the application space.
	*/
	AckMsg( const AckMsg& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~AckMsg();
	//@}

	///@name Accessors
	//@{
	/** Returns the NackCode value as a string format.
		@return string representation of this object's NackCode
	*/
	const EmaString& getNackCodeAsString() const;

	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::AckMsgEnum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::NoCodeEnum
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EmaBuffer with the message hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Indicates presence of SeqNum.
		\remark sequence number is an optional member of AckMsg
		@return true if sequence number is set; false otherwise
	*/
	bool hasSeqNum() const;

	/** Indicates presence of NackCode.
		\remark negative acknowledgement code is an optional member of AckMsg
		@return true if NackCode is set; false otherwise
	*/
	bool hasNackCode() const;

	/** Indicates presence of Text.
		\remark Text is an optional member of AckMsg
		@return true if text is set; false otherwise
	*/
	bool hasText() const;

	/** Indicates presence of the ServiceName within the MsgKey.
		\remark service name is an optional member of AckMsg
		@return true if service name is set; false otherwise
	*/
	bool hasServiceName() const;

	/** Returns SeqNum.
		@throw OmmInvalidUsageException if hasSeqNum() returns false
		@return sequence number
	*/
	UInt32 getSeqNum() const;

	/** Returns the AckId.
		@return ack id
	*/
	UInt32 getAckId() const;

	/** Returns NackCode.
		@throw OmmInvalidUsageException if hasNackCode() returns false
		@return NackCode
	*/
	UInt8 getNackCode() const;

	/** Returns Text.
		@throw OmmInvalidUsageException if hasText() returns false
		@return text
	*/
	const EmaString& getText() const;

	/** Returns PrivateStream.
		@return true if this is a private stream; false otherwise
	*/
	bool getPrivateStream() const;

	/** Returns the ServiceName within the MsgKey.
		@throw OmmInvalidUsageException if hasServiceName() returns false
		@return EmaString containing service name
	*/
	const EmaString& getServiceName() const;
	//@}

	///@name Operations
	//@{
	/** Clears the AckMsg.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	AckMsg& clear();

	/** Specifies StreamId.
		@param[in] streamId specifies stream id
		@return reference to this object
	*/
	AckMsg& streamId( Int32 streamId );

	/** Specifies DomainType.
		@throw OmmUnsupportedDomainTypeException if domainType is greater than 255
		@param[in] domainType specifies RDM Message Model Type (default value is rdm::MMT_MARKET_PRICE)
		@return reference to this object
	*/
	AckMsg& domainType( UInt16 domainType = rdm::MMT_MARKET_PRICE );

	/** Specifies Name.
		@param[in] name specifies item name
		@return reference to this object
	*/
	AckMsg& name( const EmaString& name );

	/** Specifies NameType.
		@param[in] nameType specifies RDM Instrument NameType (default value is rdm::INSTRUMENT_NAME_RIC)
		@return reference to this object
	*/
	AckMsg& nameType( UInt8 nameType = rdm::INSTRUMENT_NAME_RIC );

	/** Specifies ServiceName.
		@throw OmmInvalidUsageException if service id is already set
		@param[in] serviceName specifies service name
		@return reference to this object
	*/
	AckMsg& serviceName( const EmaString& serviceName );

	/** Specifies ServiceId.
		@throw OmmInvalidUsageException if service name is already set
		@param[in] serviceId specifies service id
		@return reference to this object
	*/
	AckMsg& serviceId( UInt32 serviceId );

	/** Specifies Id.
		@param[in] id specifies Id
		@return reference to this object
	*/
	AckMsg& id( Int32 id );

	/** Specifies Filter.
		@param[in] filter specifies filter
		@return reference to this object
	*/
	AckMsg& filter( UInt32 filter );

	/** Specifies SeqNum.
		@param[in] seqNum specifies sequence number
		@return reference to this object
	*/
	AckMsg& seqNum( UInt32 seqNum );

	/** Specifies AckId.
		@param[in] postId specifies related post id
		@return reference to this object
	*/
	AckMsg& ackId( UInt32 postId );

	/** Specifies NackCode.
		@param[in] nackCode a negative acknowledgement code
		@return reference to this object
	*/
	AckMsg& nackCode( UInt8 nackCode );

	/** Specifies Text.
		@param[in] text specifies message text information
		@return reference to this object
	*/
	AckMsg& text( const EmaString& text );

	/** Specifies Attrib.
		@param[in] attrib an object of ComplexType
		@return reference to this object
	*/
	AckMsg& attrib( const ComplexType& attrib );

	/** Specifies Payload.
		@param[in] payload an object of ComplexType
		@return reference to this object
	*/
	AckMsg& payload( const ComplexType& payload );

	/** Specifies ExtendedHeader.
		@param[in] buffer an EmaBuffer containing extendedHeader information
		@return reference to this object
	*/
	AckMsg& extendedHeader( const EmaBuffer& buffer );

	/** Specifies PrivateStream.
		@param[in] privateStream specifies if this is a private stream (default is false)
		@return reference to this object
	*/
	AckMsg& privateStream( bool privateStream = false );
	//@}

private :

	friend class ItemCallbackClient;

	const EmaString& toString( UInt64 indent ) const;

	Decoder& getDecoder();

	AckMsg& operator=( const AckMsg& );

	mutable EmaString		_toString;
};

}

}

}

#endif // __thomsonreuters_ema_access_AckMsg_h
