/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_GenericMsg_h
#define __thomsonreuters_ema_access_GenericMsg_h

/**
	@class thomsonreuters::ema::access::GenericMsg GenericMsg.h "Access/Include/GenericMsg.h"
	@brief GenericMsg allows applications to bidirectionally send messages without any implied
	message semantics.

	GenericMsg may be sent on any item stream using OmmConsumer::submit( const GenericMsg& , UInt64 ).

	The following code snippet shows processing of received GenericMsg.

	\code

	class AppClient : public OmmConsumerClient
	{
		...

		void onGenericMsg( const GenericMsg& , const OmmConsumerEvent& );
	};

	void AppClient::onGenericMsg( const GenericMsg& genMsg, const OmmConsumerEvent& )
	{
		if ( genMsg.hasName() )
			cout << genMsg.getName() << "\n";

		switch ( gemMsg.getPayload().getDataTyoe() )
		{
			case DataType::FieldListEnum :
				decode( genMsg.getPayload().getFieldList() );
				break;
			case DataType::NoDataEnum :
				break;
		}
	}

	\endcode

	The following code snippet shows setting and submission of GenericMsg.

	\code

	GenericMsg genMsg;
	genMsg.name( "my generic message" );

	ElementList eList;
	elist.addInt( "int", 123 ).addAscii( "ascii", "my ascii" ).complete();

	genMsg.payload( eList );
	genMsg.complete( true );

	consumer.submit( genMsg, itemHandle );

	\endcode

	\remark Calling get***() method on an optional member of GenericMsg must be preceded by a
			call to respective has***() method.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and getting of information from GenericMsg.
	\remark Objects of this class are not cache-able.
	\remark	Decoding of just encoded GenericMsg in the same application is not supported.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		Msg,
		EmaString,
		EmaBuffer,
*/

#include "Access/Include/Msg.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API GenericMsg : public Msg
{
public :

	///@name Constructor
	//@{
	/** Constructs GenericMsg.
	*/
	GenericMsg();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~GenericMsg();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::GenericMsgEnum
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

	/** Indicates presence of SeqNum.
		\remark sequence number is an optional member of GenericMsg
		@return true if sequence number is set; false otherwise
	*/
	bool hasSeqNum() const;

	/** Indicates presence of SecondarySeqNum.
		\remark secondary sequence number is an optional member of GenericMsg
		@return true if secondary sequence number is set; false otherwise
	*/
	bool hasSecondarySeqNum() const;

	/** Indicates presence of PartNum.
		\remark part number is an optional member of GenericMsg
		@return true if part number is set; false otherwise
	*/
	bool hasPartNum() const;

	/** Indicates presence of PermissionData.
		\remark permission data is optional on GenericMsg
		@return true if permission data is set; false otherwise
	*/
	bool hasPermissionData() const;

	/** Returns SeqNum.
		@throw OmmInvalidUsageException if hasSeqNum() returns false
		@return sequence number
	*/
	UInt32 getSeqNum() const;

	/** Returns SecondarySeqNum.
		@throw OmmInvalidUsageException if hasSecondarySeqNum() returns false
		@return secondary sequence number
	*/
	UInt32 getSecondarySeqNum() const;

	/** Returns PartNum.
		@throw OmmInvalidUsageException if hasPartNum() returns false
		@return part number
	*/
	UInt16 getPartNum() const;

	/** Returns PermissionData.
		@throw OmmInvalidUsageException if hasPermissionData() returns false
		@return EmaBuffer containing permission data
	*/
	const EmaBuffer& getPermissionData() const;

	/** Returns Complete.
		@return true if this is a one part generic message or the final part of the multi part generic message.
	*/
	bool getComplete() const;
	//@}

	///@name Operations
	//@{
	/** Clears the GenericMsg.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	GenericMsg& clear();

	/** Specifies StreamId.
		@param[in] streamId specifies stream id
		@return reference to this object
	*/
	GenericMsg& streamId( Int32 streamId );

	/** Specifies DomainType.
		@throw OmmUnsupportedDomainTypeException if domainType is greater than 255
		@param[in] domainType specifies RDM Message Model Type (default value is rdm::MMT_MARKET_PRICE)
		@return reference to this object
	*/
	GenericMsg& domainType( UInt16 domainType = rdm::MMT_MARKET_PRICE );

	/** Specifies Name.
		@param[in] name specifies item name
		@return reference to this object
	*/
	GenericMsg& name( const EmaString& name );

	/** Specifies NameType.
		@param[in] nameType specifies RDM Instrument NameType (default value is rdm::INSTRUMENT_NAME_RIC)
		@return reference to this object
	*/
	GenericMsg& nameType( UInt8 nameType = rdm::INSTRUMENT_NAME_RIC );

	/** Specifies ServiceId.
		@param[in] serviceId specifies service id
		@return reference to this object
	*/
	GenericMsg& serviceId( UInt32 serviceId );

	/** Specifies Id.
		@param[in] id specifies Id
		@return reference to this object
	*/
	GenericMsg& id( Int32 id );

	/** Specifies Filter.
		@param[in] filter specifies filter
		@return reference to this object
	*/
	GenericMsg& filter( UInt32 filter );

	/** Specifies SeqNum.
		@param[in] seqNum specifies sequence number
		@return reference to this object
	*/
	GenericMsg& seqNum( UInt32 seqNum );

	/** Specifies SecondarySeqNum.
		@param[in] secondarySeqNum specifies secondary sequence number
		@return reference to this object
	*/
	GenericMsg& secondarySeqNum( UInt32 secondarySeqNum );

	/** Specifies PartNum.
		@param[in] partNum specifies part number
		@return reference to this object
	*/
	GenericMsg& partNum( UInt16 partNum );

	/** Specifies PermissionData.
		@param[in] permissionData an EmaBuffer object with permission data information
		@return reference to this object
	*/
	GenericMsg& permissionData( const EmaBuffer& permissionData );

	/** Specifies Attrib.
		@param[in] attrib an object of ComplexType
		@return reference to this object
	*/
	GenericMsg& attrib( const ComplexType& data );

	/** Specifies Payload.
		@param[in] payload an object of ComplexType
		@return reference to this object
	*/
	GenericMsg& payload( const ComplexType& data );

	/** Specifies ExtendedHeader.
		@param[in] buffer an EmaBuffer containing extendedHeader information
		@return reference to this object
	*/
	GenericMsg& extendedHeader( const EmaBuffer& buffer );

	/** Specifies Complete.
		\remark must be set to true for one part generic message
		@param[in] complete specifies if this is the last part of the multi part generic message
		@return reference to this object
	*/
	GenericMsg& complete( bool complete = true );
	//@}

private :

	friend class ItemCallbackClient;

	const EmaString& toString( UInt64 indent ) const;

	Decoder& getDecoder();

	GenericMsg( const GenericMsg& );
	GenericMsg& operator=( const GenericMsg& );

	mutable EmaString		_toString;
};

}

}

}

#endif
