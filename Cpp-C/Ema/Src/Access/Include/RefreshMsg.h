/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_RefreshMsg_h
#define __refinitiv_ema_access_RefreshMsg_h

/**
	@class rtsdk::ema::access::RefreshMsg RefreshMsg.h "Access/Include/RefreshMsg.h"
	@brief RefreshMsg conveys item image, state, permission and group information.

	RefreshMsg is sent when item data needs to be synchronized. This happens as a response
	to received ReqMsg or when upstream source requires it. RefreshMsg sent as a response to
	ReqMsg is called solicited refresh while unsolicited refresh is sent when upstream source
	requires synchronization of downstream consumers.

	The following code snippet shows receiving and processing of RefreshMsg.

	\code

	class AppClient ; public OmmConsumerClient
	{
		...

		void onRefreshMsg( const RefreshMsg& , const OmmConsumerEvent& );
	};

	void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& event )
	{
		cout << "Item's handle = " << event.getHandle() << "\n";

		if ( refreshMsg.hasName() )
			cout << "Item's name = " << refreshMsg.getName() << "\n";

		if ( refreshMsg.hasServiceName() )
			cout << "Item's name = " << refreshMsg.getServiceName() << "\n";

		switch ( refreshMsg.getPayload().getDataTyoe() )
		{
			case DataType::FieldListEnum :
				decode( refreshMsg.getPayload().getFieldList() );
				break;
			case DataType::NoDataEnum :
				break;
		}
	}

	\endcode
			
	\remark Calling get***() method on an optional member of RefreshMsg must be preceded by a
			call to respective has***() method.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and getting of information from RefreshMsg.
	\remark Objects of this class are not cache-able.
	\remark	Decoding of just encoded RefreshMsg in the same application is not supported.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		Msg,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Msg.h"
#include "Access/Include/OmmState.h"
#include "Access/Include/OmmQos.h"

namespace rtsdk {

namespace ema {

namespace access {

class EMA_ACCESS_API RefreshMsg : public Msg
{
public :

	///@name Constructor
	//@{
	/** Constructs RefreshMsg.
	 */
	RefreshMsg();

	/** Copy constructor.
		\remark this is used to copy and process RefreshMsg outside of EMA's callback methods.
		\remark this method does not support passing in just encoded RefreshMsg in the application space.
	*/
	RefreshMsg( const RefreshMsg& other );
	//@}

	///@name Constructor
	//@{
	/** Destructor.
	 */
	virtual ~RefreshMsg();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::RefeshMsgEnum
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

	/** Indicates presence of Qos.
		\remark Qos is an optional member of RefreshMsg
		@return true if Qos is set; false otherwise
	*/
	bool hasQos() const;

	/** Indicates presence of SeqNum.
		\remark sequence number (SeqNum) is an optional member of RefreshMsg
		@return true if sequence number is set; false otherwise
	*/
	bool hasSeqNum() const;

	/** Indicates presence of PartNum.
		\remark part number is an optional member of RefreshMsg
		@return true if part number is set; false otherwise
	*/
	bool hasPartNum() const;

	/** Indicates presence of PermissionData.
		\remark permission data is optional member of RefreshMsg
		@return true if permission data is set; false otherwise
	*/
	bool hasPermissionData() const;

	/** Indicates presence of PublisherId.
		\remark publisher id is an optional member of RefreshMsg
		@return true if publisher id is set; false otherwise
	*/
	bool hasPublisherId() const;

	/** Indicates presence of the ServiceName within the MsgKey.
		\remark service name is an optional member of RefreshMsg
		@return true if service name is set; false otherwise
	*/
	bool hasServiceName() const;

	/** Returns State.
		@return state of item (e.g. Open / Ok)
	*/
	const OmmState& getState() const;

	/** Returns Qos.
		@throw OmmInvalidUsageException if hasQos() returns false
		@return Qos of item
	*/
	const OmmQos& getQos() const;

	/** Returns SeqNum.
		@throw OmmInvalidUsageException if hasSeqNum() returns false
		@return sequence number
	*/
	UInt32 getSeqNum() const;

	/** Returns PartNum.
		@throw OmmInvalidUsageException if hasPartNum() returns false
		@return part number
	*/
	UInt16 getPartNum() const;

	/** Returns ItemGroup.
		@return EmaBuffer containing item group information
	*/
	const EmaBuffer& getItemGroup() const;

	/** Returns PermissionData.
		@throw OmmInvalidUsageException if hasPermissionData() returns false
		@return EmaBuffer containing permission data
	*/
	const EmaBuffer& getPermissionData() const;

	/** Returns PublisherIdUserId.
		@throw OmmInvalidUsageException if hasPublisherId() returns false
		@return publisher's user Id
	*/
	UInt32 getPublisherIdUserId() const;

	/** Returns PublisherIdUserAddress.
		@throw OmmInvalidUsageException if hasPublisherId() returns false
		@return publisher's user address
	*/
	UInt32 getPublisherIdUserAddress() const;

	/** Returns Solicited.
		@return true if this is solicited refresh; false otherwise
	*/ 
	bool getSolicited() const;
		
	/** Returns DoNotCache.
		@return true if this refresh must not be cached; false otherwise
	*/
	bool getDoNotCache() const;

	/** Returns Complete.
		@return true if this is the last part of the multi part refresh message
	*/
	bool getComplete() const;

	/** Returns ClearCache.
		@return true if cache needs to be cleared before applying this refresh; false otherwise
	*/
	bool getClearCache() const;

	/** Returns PrivateStream.
		@retturn true if this is private stream item
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
	/** Clears the RefreshMsg.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	RefreshMsg& clear();

	/** Specifies StreamId.
		@param[in] id stream id
		@return reference to this object
	*/
	RefreshMsg& streamId( Int32 streamId );

	/** Specifies DomainType.
		@throw OmmUnsupportedDomainType if domainType is greater than 255
		@param[in] domainType specifies RDM Message Model Type (default value is rdm::MMT_MARKET_PRICE)
		@return reference to this object
	*/
	RefreshMsg& domainType( UInt16 domainType = rdm::MMT_MARKET_PRICE );

	/** Specifies Name.
		@param[in] name an EmaString object containing item name
		@return reference to this object
	*/
	RefreshMsg& name( const EmaString& name );

	/** Specifies NameType.
		@param[in] nameType specifies RDM Instrument NameType (default value is rdm::INSTRUMENT_NAME_RIC)
		@return reference to this object
	*/
	RefreshMsg& nameType( UInt8 nameType = rdm::INSTRUMENT_NAME_RIC );

	/** Specifies ServiceName.
		@throw OmmInvalidUsageException if service id is already set
		@param[in] serviceName an EmaString object containing service name
		@return reference to this object
	*/
	RefreshMsg& serviceName( const EmaString& serviceName );

	/** Specifies ServiceId.
		@throw OmmInvalidUsageException if service name is already set
		@param[in] serviceId service identifier
		@return reference to this object
	*/
	RefreshMsg& serviceId( UInt32 serviceId );

	/** Specifies Id.
		@param[in] id specifies Id
		@return reference to this object
	*/
	RefreshMsg& id( Int32 id );

	/** Specifies Filter.
		@param[in] filter specifies Filter
		@return reference to this object
	*/
	RefreshMsg& filter( UInt32 filter );

	/** Specifies Qos.
		@param[in] timeliness specifies Qos Timeliness (default value is OmmQos::RealTimeEnum)
		@param[in] rate specifies Qos rate (default value is OmmQos::TickByTickEnum)
		@return reference to this object
	*/
	RefreshMsg& qos( UInt32 timeliness = OmmQos::RealTimeEnum, UInt32 rate = OmmQos::TickByTickEnum );

	/** Specifies State.
		@param[in] streamState conveys item stream state value (default value is OmmState::OpenEnum)
		@param[in] dataState conveys item data state value (default value is OmmState::SuspectEnum)
		@param[in] statusCode conveys specific item state code (default value is OmmState::NoneEnum)
		@param[in] statusText conveys item status explanation (default value is 'empty string')
		@return reference to this object
	*/
	RefreshMsg& state( OmmState::StreamState streamState = OmmState::OpenEnum,
		OmmState::DataState dataState = OmmState::OkEnum,
		UInt8 statusCode = 0,
		const EmaString& statusText = EmaString() );

	/** Specifies SeqNum.
		@param[in] seqNum specifies sequence number
		@return reference to this object
	*/
	RefreshMsg& seqNum( UInt32 seqNum );

	/** Specifies PartNum.
		@param[in] partNum specifies part number
		@return reference to this object
	*/
	RefreshMsg& partNum( UInt16 partNum );

	/** Specifies ItemGroup.
		@param[in] itemGroup an EmaBuffer object with item group information
		@return reference to this object
	*/
	RefreshMsg& itemGroup( const EmaBuffer& itemGroup );

	/** Specifies PermissionData.
		@param[in] permissionData an EmaBuffer object with permission data information
		@return reference to this object
	*/
	RefreshMsg& permissionData( const EmaBuffer& permissionData );

	/** Specifies PublisherId.
		@param[in] UserId specifies publisher's user id
		@param[in] UserAddress specifies publisher's user address
		@return reference to this object
	*/
	RefreshMsg& publisherId( UInt32 userId, UInt32 userAddress );

	/** Specifies Attrib.
		@param[in] attrib an object of ComplexType
		@return reference to this object
	*/
	RefreshMsg& attrib( const ComplexType& data );

	/** Specifies Payload.
		@param[in] payload an object of ComplexType
		@return reference to this object
	*/
	RefreshMsg& payload( const ComplexType& data );

	/** Specifies ExtendedHeader.
		@param[in] buffer an EmaBuffer containing extendedHeader information
		@return reference to this object
	*/
	RefreshMsg& extendedHeader( const EmaBuffer& buffer );

	/** Specifies Solicited.
		@param[in] solicited true if this refresh is solicited; false otherwise (default value is false)
		@return reference to this object
	*/
	RefreshMsg& solicited( bool solicited = false );
		
	/** Specifies DoNotCache.
		@param[in] doNotCache true if this refresh must not be cached; false otherwise (default value is false)
		@return reference to this object
	*/
	RefreshMsg& doNotCache( bool doNotCache = false );

	/** Specifies ClearCache.
		@param[in] clearCache true if cache needs to be cleared; false otherwise (default value is false)
		@return reference to this object
	*/
	RefreshMsg& clearCache( bool clearCache = false );

	/** Specifies RefreshComplete.
		@param[in] complete true if this is the last part of multi part refresh or single part refresh; false otherwise (default value is true)
		@return reference to this object
	*/
	RefreshMsg& complete( bool complete = true );

	/** Specifies PrivateStream.
		@param[in] privateStream true if private stream; false otherwise (default value is false)
		@return reference to this object
	*/
	RefreshMsg& privateStream( bool privateStream = false );
	//@}

private :

	friend class ItemCallbackClient;
	friend class DictionaryCallbackClient;

	const EmaString& toString( UInt64 ) const;

	Decoder& getDecoder();

	RefreshMsg& operator=( const RefreshMsg& );

	mutable EmaString		_toString;
};

}

}

}

#endif // __refinitiv_ema_access_RefreshMsg_h
