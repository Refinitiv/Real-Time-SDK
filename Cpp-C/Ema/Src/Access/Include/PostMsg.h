/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_PostMsg_h
#define __rtsdk_ema_access_PostMsg_h

/**
	@class rtsdk::ema::access::PostMsg PostMsg.h "Access/Include/PostMsg.h"
	@brief PostMsg allows consumer applications to contribute content.

	PostMsg may be submitted on any market item stream or login stream. Submission on
	a market item stream is referred to as the "on stream posting" while submission on
	a login stream is considered as the "off stream posting". On stream posting content
	is related to the item on whose stream the posting happens, while the off stream posting
	may contain info about any item.

	PostMsg may be submitted using OmmConsumer::submit( const PostMsg& , UInt64 ).

	The following code snippet shows setting and submission of an on stream PostMsg.

	\code

	PostMsg postMsg;

	FieldList fieldList;
	fieldList.addInt( 12, 123 ).addAscii( 255, "my ascii" ).complete();

	postMsg.payload( fieldList );

	consumer.submit( postMsg, itemHandle );

	\endcode

	\remark Calling get***() method on an optional member of PostMsg must be preceded by a
			call to respective has***() method.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and getting of information from PostMsg.
	\remark Objects of this class are not cache-able.
	\remark	Decoding of just encoded PostMsg in the same application is not supported.
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

class EMA_ACCESS_API PostMsg : public Msg
{
public :

	/** @enum PostUserRights
		An enumeration representing post user rights.
	*/
	enum PostUserRights
	{
		CreateEnum = 0x01,					/*!< Specifies ability to create records. */

		DeleteEnum = 0x02,					/*!< Specifies ability to delete records. */

		ModifyPermissionEnum = 0x04			/*!< Specifies ability to modify permissions. */
	};

	///@name Constructor
	//@{
	/** Constructs PostMsg.
	 */
	PostMsg();

	/** Copy constructor.
		\remark this is used to copy and process PostMsg outside of EMA's callback methods.
		\remark this method does not support passing in just encoded PostMsg in the application space.
	*/
	PostMsg( const PostMsg& other );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	 */
	virtual ~PostMsg();
	//@}		

	///@name Accessors
	//@{
	/** Returns the PostUserRights value as a string format.
		@return string representation of PostUserRights
	*/
	const EmaString& getPostUserRightsAsString() const;
	
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::PostMsgEnum
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
		\remark sequence number is an optional member of PostMsg
		@return true if sequence number is set; false otherwise
	*/
	bool hasSeqNum() const;

	/** Indicates presence of the PostId.
		\remark post id is an optional member of PostMsg
		@return true if PostId is set; false otherwise
	*/
	bool hasPostId() const;

	/** Indicates presence of PartNum.
		\remark part number is an optional member of PostMsg
		@return true if part number is set; false otherwise
	*/
	bool hasPartNum() const;

	/** Indicates presence of PostUserRights.
		\remark post user rights is an optional member of PostMsg
		@return true if PostUserRights are set; false otherwise
	*/
	bool hasPostUserRights() const;

	/** Indicates presence of PermissionData.
		\remark permission data is an optional member of PostMsg
		@return true if permission data is set; false otherwise
	 */
	bool hasPermissionData() const;

	/** Indicates presence of the ServiceName within the MsgKey.
		\remark service name is an optional member of PostMsg
		@return true if service name is set; false otherwise
	*/
	bool hasServiceName() const;

	/** Returns SeqNum.
		@throw OmmInvalidUsageException if hasSeqNum() returns false
		@return sequence number
	*/
	UInt32 getSeqNum() const;

	/** Returns the PostId.
		@throw OmmInvalidUsageException if hasPostId() returns false
		@return post id
	*/
	UInt32 getPostId() const;

	/** Returns PartNum.
		@throw OmmInvalidUsageException if hasPartNum() returns false
		@return part number
	*/
	UInt16 getPartNum() const;

	/** Returns PostUserRights.
		@throw OmmInvalidUsageException if hasPostUserRights() returns false
		@return PostUserRights
	*/
	UInt16 getPostUserRights() const;

	/** Returns PermissionData.
		@throw OmmInvalidUsageException if hasPermissionData() returns false
		@return EmaBuffer containing permission data
	*/
	const EmaBuffer& getPermissionData() const;

	/** Returns PublisherIdUserId.
		@return publisher's user Id
	*/
	UInt32 getPublisherIdUserId() const;

	/** Returns PublisherIdUserAddress.
		@return publisher's user address
	*/
	UInt32 getPublisherIdUserAddress() const;

	/** Indicates that acknowledgement is requested.
		@return true if acknowledgement is requested; false otherwise
	*/
	bool getSolicitAck() const;

	/** Returns Complete.
		@return true if this is the last part of the multi part post message
	*/
	bool getComplete() const;

	/** Returns the ServiceName within the MsgKey.
		@throw OmmInvalidUsageException if hasServiceName() returns false
		@return EmaString containing service name
	*/
	const EmaString& getServiceName() const;
	//@}

	///@name Operations
	//@{
	/** Clears the PostMsg.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	PostMsg& clear();

	/** Specifies StreamId.
		@param[in] streamId specifies stream id
		@return reference to this object
	*/
	PostMsg& streamId( Int32 streamId );

	/** Specifies DomainType.
		@throw OmmUnsupportedDomainTypeException if domainType is greater than 255
		@param[in] domainType specifies RDM Message Model Type (default value is rdm::MMT_MARKET_PRICE)
		@return reference to this object
	*/
	PostMsg& domainType( UInt16 domainType = rdm::MMT_MARKET_PRICE );

	/** Specifies Name.
		@param[in] name specifies item name
		@return reference to this object
	*/
	PostMsg& name( const EmaString& name );

	/** Specifies NameType.
		@param[in] nameType specifies RDM Instrument NameType (default value is rdm::INSTRUMENT_NAME_RIC)
		@return reference to this object
	*/
	PostMsg& nameType( UInt8 nameType = rdm::INSTRUMENT_NAME_RIC );

	/** Specifies ServiceName.
		@throw OmmInvalidUsageException if service id is already set
		@param[in] serviceName specifies service name
		@return reference to this object
	*/
	PostMsg& serviceName( const EmaString& name );

	/** Specifies ServiceId.
		@throw OmmInvalidUsageException if service name is already set
		@param[in] serviceId specifies service id
		@return reference to this object
	*/
	PostMsg& serviceId( UInt32 serviceId );

	/** Specifies Id.
		@param[in] id specifies Id
		@return reference to this object
	*/
	PostMsg& id( Int32 id );

	/** Specifies Filter.
		@param[in] filter specifies filter
		@return reference to this object
	*/
	PostMsg& filter( UInt32 filter );

	/** Specifies SeqNum.
		@param[in] seqNum specifies sequence number
		@return reference to this object
	*/
	PostMsg& seqNum( UInt32 seqNum );

	/** Specifies PostId.
		@param[in] postId specifies post id
		@return reference to this object
	*/
	PostMsg& postId( UInt32 postId );

	/** Specifies PartNum.
		@param[in] partNum specifies part number
		@return reference to this object
	*/
	PostMsg& partNum( UInt16 partNum );

	/** Specifies PostUserRights.
		@param[in] postUserRights specifies post user rights
		@return reference to this object
	*/
	PostMsg& postUserRights( UInt16 postUserRights );

	/** Specifies PermissionData.
		@param[in] permissionData an EmaBuffer object with permission data information
		@return reference to this object
	*/
	PostMsg& permissionData( const EmaBuffer& permissionData );

	/** Specifies PublisherId.
		@param[in] UserId specifies publisher's user id
		@param[in] UserAddress specifies publisher's user address
		@return reference to this object
	*/
	PostMsg& publisherId( UInt32 UserId, UInt32 UserAddress );

	/** Specifies Attrib.
		@param[in] attrib an object of ComplexType
		@return reference to this object
	*/
	PostMsg& attrib( const ComplexType& data );

	/** Specifies Payload.
		@param[in] payload an object of ComplexType
		@return reference to this object
	*/
	PostMsg& payload( const ComplexType& data );

	/** Specifies ExtendedHeader.
		@param[in] buffer an EmaBuffer containing extendedHeader information
		@return reference to this object
	*/
	PostMsg& extendedHeader( const EmaBuffer& buffer );

	/** Specifies acknowledgement.
		@param[in] ack specifies if an acknowledgement is requested (default value is false)
		@return reference to this object
	*/
	PostMsg& solicitAck( bool ack = false );

	/** Specifies Complete.
		\remark must be set to true for one part post message
		@param[in] complete specifies if this is the last part of the multi part post message
		@return reference to this object
	*/
	PostMsg& complete( bool complete = true );
	//@}

private :

	mutable EmaString		_toString;
	mutable EmaString		_postUserRightsString;

	const EmaString& toString( UInt64 ) const;

	Decoder& getDecoder();

	PostMsg& operator=( const PostMsg& );
};

}

}

}

#endif // __rtsdk_ema_access_PostMsg_h
